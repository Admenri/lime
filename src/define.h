#pragma once

// Klass attribute
#define ATTR(ty, name) \
  std::optional<ty> Attr_##name(std::optional<ty> value = std::nullopt)
#define ATTR_DEF(ty, name, klass) \
  std::optional<ty> klass::Attr_##name(std::optional<ty> value)

// Klass serialization
#define MARSHAL_DUMP(ty) static std::string MarshalDump(RefPtr<ty> obj);
#define MARSHAL_DUMP_DEF(ty) std::string ty::MarshalDump(RefPtr<ty> obj)
#define MARSHAL_LOAD(ty) static RefPtr<ty> MarshalLoad(std::string data);
#define MARSHAL_LOAD_DEF(ty) RefPtr<ty> ty::MarshalLoad(std::string data)
