// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <istream>
#include <iterator>
#include <memory>
#include <ostream>

#include "src/common.h"

namespace rgssx {

class IOStream {
 public:
  IOStream(void* ptr);
  ~IOStream();

  int64_t Tell();
  int32_t Seek(uint64_t pos);
  int64_t Read(void* buffer, uint32_t size);
  int64_t Write(const void* buffer, uint32_t size);
  int64_t Length();

  std::string ReadAll();

 private:
  void* ptr_;
};

class IOService : public Singleton<IOService> {
 public:
  IOService(const std::string& argv0);
  ~IOService();

  // Write output path
  bool SetWritePath(const std::string& path);

  // Loading path
  int32_t AddLoadPath(const std::string& new_path,
                      const std::string& mount_point,
                      bool append = true);
  int32_t RemoveLoadPath(const std::string& old_path);
  bool Exists(const std::string& filename);
  std::vector<std::string> EnumDir(const std::string& dir);

  // Returns true if the given path exists and is a directory.
  bool IsDirectory(const std::string& path);
  // Creates a new directory (including any missing parent directories).
  // Returns true on success, false on failure.
  bool Mkdir(const std::string& path);
  // Removes an empty directory. Returns true on success, false on failure.
  bool Rmdir(const std::string& path);

  std::string GetLastError();

  // std::stream output
  using OpenCallback =
      std::function<bool(std::unique_ptr<IOStream>, const std::string&)>;
  void OpenRead(const std::string& file_path, OpenCallback callback);
  std::unique_ptr<IOStream> OpenReadRaw(const std::string& filename);
  std::unique_ptr<IOStream> OpenWrite(const std::string& filename);
};

}  // namespace rgssx
