// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <functional>
#include <istream>
#include <iterator>
#include <memory>
#include <ostream>

#include "src/common.h"

namespace lime {

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

}  // namespace lime
