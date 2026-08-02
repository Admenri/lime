// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "filesystem.h"

#include <algorithm>
#include <cctype>
#include <istream>
#include <memory>
#include <ostream>

#include "physfs.h"

namespace rgssx {

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

// std::streambuf adapter that forwards all IO to a PhysicsFS file handle.
class PHYSFSStreamBuf : public std::streambuf {
 public:
  explicit PHYSFSStreamBuf(PHYSFS_File* file) : file_(file) {}

  ~PHYSFSStreamBuf() override {
    if (file_)
      PHYSFS_close(file_);
  }

  // Read -------------------------------------------------------------
  std::streamsize xsgetn(char_type* s, std::streamsize n) override {
    if (!file_)
      return 0;

    PHYSFS_sint64 result = PHYSFS_readBytes(file_, s, n);
    return (result != -1) ? result : 0;
  }

  int_type underflow() override {
    if (!file_)
      return traits_type::eof();

    char_type c;
    PHYSFS_sint64 result = PHYSFS_readBytes(file_, &c, 1);
    if (result != 1)
      return traits_type::eof();

    // Rewind so the byte is not consumed
    PHYSFS_seek(file_, PHYSFS_tell(file_) - 1);
    return traits_type::to_int_type(c);
  }

  int_type uflow() override {
    if (!file_)
      return traits_type::eof();

    char_type c;
    PHYSFS_sint64 result = PHYSFS_readBytes(file_, &c, 1);
    return (result == 1) ? traits_type::to_int_type(c) : traits_type::eof();
  }

  // Write ------------------------------------------------------------
  std::streamsize xsputn(const char_type* s, std::streamsize n) override {
    if (!file_)
      return 0;

    PHYSFS_sint64 result = PHYSFS_writeBytes(file_, s, n);
    return (result != -1) ? result : 0;
  }

  int_type overflow(int_type ch) override {
    if (!file_)
      return traits_type::eof();

    if (!traits_type::eq_int_type(ch, traits_type::eof())) {
      char_type c = traits_type::to_char_type(ch);
      PHYSFS_sint64 result = PHYSFS_writeBytes(file_, &c, 1);
      if (result != 1)
        return traits_type::eof();
    }

    return traits_type::not_eof(ch);
  }

  // Positioning ------------------------------------------------------
  pos_type seekoff(off_type off,
                   std::ios_base::seekdir dir,
                   std::ios_base::openmode which) override {
    if (!file_)
      return pos_type(off_type(-1));

    int64_t base;
    switch (dir) {
      case std::ios_base::beg:
        base = 0;
        break;
      case std::ios_base::cur:
        base = PHYSFS_tell(file_);
        break;
      case std::ios_base::end:
        base = PHYSFS_fileLength(file_);
        break;
      default:
        return pos_type(off_type(-1));
    }

    if (!PHYSFS_seek(file_, base + off))
      return pos_type(off_type(-1));

    return pos_type(PHYSFS_tell(file_));
  }

  pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
    return seekoff(off_type(pos), std::ios_base::beg, which);
  }

  std::streamsize showmanyc() override {
    if (!file_)
      return 0;

    PHYSFS_sint64 length = PHYSFS_fileLength(file_);
    PHYSFS_sint64 pos = PHYSFS_tell(file_);
    return static_cast<std::streamsize>(
        std::max<PHYSFS_sint64>(0, length - pos));
  }

 private:
  PHYSFS_File* file_ = nullptr;
};

// RAII streams owning their streambuf (and therefore the file handle).
class InputFileStream : public std::istream {
 public:
  explicit InputFileStream(PHYSFS_File* file) : std::istream(nullptr) {
    buf_ = std::make_unique<PHYSFSStreamBuf>(file);
    rdbuf(buf_.get());
  }

 private:
  std::unique_ptr<PHYSFSStreamBuf> buf_;
};

class OutputFileStream : public std::ostream {
 public:
  explicit OutputFileStream(PHYSFS_File* file) : std::ostream(nullptr) {
    buf_ = std::make_unique<PHYSFSStreamBuf>(file);
    rdbuf(buf_.get());
  }

 private:
  std::unique_ptr<PHYSFSStreamBuf> buf_;
};

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
  auto stream = std::make_unique<InputFileStream>(file);
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
    throw std::runtime_error(data.physfs_error + ": " + file_path);

  if (data.match_count <= 0)
    throw std::runtime_error("No file match: " + file_path);
}

std::unique_ptr<std::istream> IOService::OpenReadRaw(
    const std::string& filename) {
  PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
  if (!file)
    throw std::runtime_error(
        std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())) + ": " +
        filename);

  return std::make_unique<InputFileStream>(file);
}

std::unique_ptr<std::ostream> IOService::OpenWrite(
    const std::string& filename) {
  PHYSFS_File* file = PHYSFS_openWrite(filename.c_str());
  if (!file)
    throw std::runtime_error(
        std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())) + ": " +
        filename);

  return std::make_unique<OutputFileStream>(file);
}

}  // namespace rgssx
