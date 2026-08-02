// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <istream>
#include <iterator>
#include <memory>
#include <ostream>

#include "common.h"

namespace rgssx {

// Reads the entire stream content into a string (may hold binary data).
inline std::string ReadStream(std::istream& stream) {
  return std::string((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
}

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

  std::string GetLastError();

  // std::stream output
  using OpenCallback =
      std::function<bool(std::unique_ptr<std::istream>, const std::string&)>;
  void OpenRead(const std::string& file_path, OpenCallback callback);
  std::unique_ptr<std::istream> OpenReadRaw(const std::string& filename);
  std::unique_ptr<std::ostream> OpenWrite(const std::string& filename);
};

}  // namespace rgssx
