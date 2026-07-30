#include "win32.h"

#include <string>
#include <vector>

#include <windows.h>

namespace {

const char* kRegPath = "SOFTWARE\\Enterbrain\\RGSS3\\RTP";

bool OpenRegistryKey(HKEY root, const char* subKey, HKEY* hKey) {
  LONG ret = RegOpenKeyExA(root, subKey, 0, KEY_READ | KEY_WOW64_32KEY, hKey);

  if (ret == ERROR_SUCCESS)
    return true;

  ret = RegOpenKeyExA(root, subKey, 0, KEY_READ | KEY_WOW64_64KEY, hKey);

  return ret == ERROR_SUCCESS;
}

bool ReadRegistryString(HKEY root,
                        const char* subKey,
                        const char* valueName,
                        std::string& value) {
  HKEY hKey = NULL;

  if (!OpenRegistryKey(root, subKey, &hKey))
    return false;

  DWORD type = 0;
  DWORD size = 0;

  LONG ret = RegQueryValueExA(hKey, valueName, NULL, &type, NULL, &size);

  if (ret != ERROR_SUCCESS) {
    RegCloseKey(hKey);
    return false;
  }

  if (type != REG_SZ && type != REG_EXPAND_SZ) {
    RegCloseKey(hKey);
    return false;
  }

  std::vector<char> buffer(size);

  ret = RegQueryValueExA(hKey, valueName, NULL, &type,
                         reinterpret_cast<LPBYTE>(buffer.data()), &size);

  RegCloseKey(hKey);

  if (ret != ERROR_SUCCESS)
    return false;

  value.assign(buffer.data());

  if (type == REG_EXPAND_SZ) {
    DWORD len = ExpandEnvironmentStringsA(value.c_str(), NULL, 0);

    if (len > 0) {
      std::vector<char> expanded(len);

      ExpandEnvironmentStringsA(value.c_str(), expanded.data(), len);

      value.assign(expanded.data());
    }
  }

  return true;
}

}  // namespace

namespace platform {
namespace win32 {

std::optional<std::string> GetRTPPath(int version, std::string key) {
  std::string value;
  auto result =
      ReadRegistryString(HKEY_LOCAL_MACHINE, kRegPath, key.c_str(), value);
  if (result) {
    return value;
  } else {
    return std::nullopt;
  }
}

}  // namespace win32
}  // namespace platform
