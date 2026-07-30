#pragma once

#include <optional>
#include <string>

namespace platform {
namespace win32 {

std::optional<std::string> GetRTPPath(int version, std::string key);

}  // namespace win32
}  // namespace platform
