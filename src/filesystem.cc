// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "src/filesystem.h"

#include <algorithm>
#include <cctype>
#include <istream>
#include <memory>
#include <ostream>

#include "physfs.h"

namespace lime {

namespace {

void ToLower(std::string& str) {
  for (size_t i = 0; i < str.size(); ++i)
    str[i] = tolower(str[i]);
}

const char* FindFileExtName(const char* filename) {
  for (size_t i = strlen(filename); i > 0; --i) {
    if (filename[i] == '/')
      break;
    if (filename[i] == '.')
      return filename + i + 1;
  }

  return nullptr;
}

struct OpenReadEnumData {
  IOService::OpenCallback callback;
  std::string full_path;
  std::string dir_path;
  std::string file_name;
  size_t last_dot = std::string::npos;

  int match_count = 0;
  std::string physfs_error;

  OpenReadEnumData() = default;
};

PHYSFS_EnumerateCallbackResult OpenReadEnumCallback(void* data,
                                                    const char* origdir,
                                                    const char* fname) {
  OpenReadEnumData* enum_data = static_cast<OpenReadEnumData*>(data);
  std::string filename(fname);

  // Windows is case sensitive.
  // The best approach is to emulate this behavior on other operating systems.
  ToLower(filename);

  if (filename != enum_data->file_name) {
    // Match filename without extname
    std::string filename_noext = filename.substr(0, filename.rfind('.'));
    if (filename_noext != enum_data->file_name) {
      // Without extname mismatch
      return PHYSFS_ENUM_OK;
    }
  }

  std::string fullpath;
  if (*origdir) {
    fullpath += std::string(origdir);
    fullpath += "/";
  }
  fullpath += fname;

  PHYSFS_File* file = PHYSFS_openRead(fullpath.c_str());
  if (!file) {
    enum_data->physfs_error = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    return PHYSFS_ENUM_ERROR;
  }

  // Ownership of the stream (and thus the file) is transferred to the callback
  auto stream = std::make_unique<IOStream>(file);
  if (enum_data->callback(std::move(stream),
                          FindFileExtName(filename.c_str()))) {
    // Matched and stop
    enum_data->match_count++;
    return PHYSFS_ENUM_STOP;
  }

  enum_data->match_count++;
  return PHYSFS_ENUM_OK;
}

}  // namespace

IOStream::IOStream(void* ptr) : ptr_(ptr) {}

IOStream::~IOStream() {
  PHYSFS_close(static_cast<PHYSFS_file*>(ptr_));
}

int64_t IOStream::Tell() {
  return PHYSFS_tell(static_cast<PHYSFS_file*>(ptr_));
}

int32_t IOStream::Seek(uint64_t pos) {
  return PHYSFS_seek(static_cast<PHYSFS_file*>(ptr_), pos);
}

int64_t IOStream::Read(void* buffer, uint32_t size) {
  return PHYSFS_readBytes(static_cast<PHYSFS_file*>(ptr_), buffer, size);
}

int64_t IOStream::Write(const void* buffer, uint32_t size) {
  return PHYSFS_writeBytes(static_cast<PHYSFS_file*>(ptr_), buffer, size);
}

int64_t IOStream::Length() {
  return PHYSFS_fileLength(static_cast<PHYSFS_file*>(ptr_));
}

std::string IOStream::ReadAll() {
  std::string data(Length(), 0);
  auto pos = Tell();
  Seek(0);
  Read(data.data(), Length());
  Seek(pos);
  return data;
}

// ----------------------------------------------------------------

IOService::IOService(const std::string& argv0) {
  const char* init_data = argv0.c_str();

  PHYSFS_init(init_data);
}

IOService::~IOService() {
  PHYSFS_deinit();
}

bool IOService::SetWritePath(const std::string& path) {
  // Setup write output path
  return !!PHYSFS_setWriteDir(path.c_str());
}

int32_t IOService::AddLoadPath(const std::string& new_path,
                               const std::string& mount_point,
                               bool append) {
  return PHYSFS_mount(new_path.c_str(), mount_point.c_str(), append);
}

int32_t IOService::RemoveLoadPath(const std::string& old_path) {
  return PHYSFS_unmount(old_path.c_str());
}

bool IOService::Exists(const std::string& filename) {
  return PHYSFS_exists(filename.c_str());
}

std::vector<std::string> IOService::EnumDir(const std::string& dir) {
  std::vector<std::string> files;

  PHYSFS_enumerate(
      dir.c_str(),
      [](void* data, const char* origdir,
         const char* fname) -> PHYSFS_EnumerateCallbackResult {
        std::vector<std::string>* files =
            static_cast<std::vector<std::string>*>(data);
        files->push_back(fname);
        return PHYSFS_ENUM_OK;
      },
      &files);

  return files;
}

bool IOService::IsDirectory(const std::string& path) {
  PHYSFS_Stat stat;
  if (!PHYSFS_stat(path.c_str(), &stat))
    return false;
  return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
}

bool IOService::Mkdir(const std::string& path) {
  return PHYSFS_mkdir(path.c_str()) != 0;
}

bool IOService::Rmdir(const std::string& path) {
  return PHYSFS_delete(path.c_str()) != 0;
}

std::string IOService::GetLastError() {
  return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
}

void IOService::OpenRead(const std::string& file_path, OpenCallback callback) {
  std::string dir, file;
  const size_t last_slash_pos = file_path.find_last_of('/');
  if (last_slash_pos != std::string::npos) {
    dir = file_path.substr(0, last_slash_pos);
    file = file_path.substr(last_slash_pos + 1);
  } else {
    // Dir = ""
    file = file_path;
  }

  OpenReadEnumData data;
  data.callback = callback;
  data.full_path = file_path;
  data.dir_path = dir;
  data.file_name = file;
  ToLower(data.file_name);
  data.last_dot = file.rfind('.');

  PHYSFS_enumerate(dir.c_str(), OpenReadEnumCallback, &data);

  if (!data.physfs_error.empty())
    throw Exception(Exception::IOError, "{}: {}", data.physfs_error, file_path);

  if (data.match_count <= 0)
    throw Exception(Exception::IOError, "No file match: {}", file_path);
}

std::unique_ptr<IOStream> IOService::OpenReadRaw(const std::string& filename) {
  PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
  if (!file) {
    std::string error_message =
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    throw Exception(Exception::IOError, "{}: {}", error_message, filename);
  }

  return std::make_unique<IOStream>(file);
}

std::unique_ptr<IOStream> IOService::OpenWrite(const std::string& filename) {
  PHYSFS_File* file = PHYSFS_openWrite(filename.c_str());
  if (!file) {
    std::string error_message =
        PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    throw Exception(Exception::IOError, "{}: {}", error_message, filename);
  }

  return std::make_unique<IOStream>(file);
}

}  // namespace lime
