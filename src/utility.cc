#include "src/utility.h"

namespace rgssx {

MARSHAL_DUMP_DEF(Rect) {
  std::string serial_data(sizeof(int) * 4, 0);
  std::memcpy(serial_data.data() + 0 * sizeof(int32_t), &obj->x,
              sizeof(int32_t));
  std::memcpy(serial_data.data() + 1 * sizeof(int32_t), &obj->y,
              sizeof(int32_t));
  std::memcpy(serial_data.data() + 2 * sizeof(int32_t), &obj->width,
              sizeof(int32_t));
  std::memcpy(serial_data.data() + 3 * sizeof(int32_t), &obj->height,
              sizeof(int32_t));
  return serial_data;
}

MARSHAL_LOAD_DEF(Rect) {
  const size_t size = data.size();
  if (size < sizeof(int32_t) * 4)
    throw Exception("invalid data length, size: {}", size);

  const int32_t* ptr = reinterpret_cast<const int32_t*>(data.data());
  return MakeRefCounted<Rect>(*(ptr + 0), *(ptr + 1), *(ptr + 2), *(ptr + 3));
}

MARSHAL_DUMP_DEF(Color) {
  std::string serial_data(sizeof(double) * 4, 0);

  double* target_ptr = reinterpret_cast<double*>(serial_data.data());
  *(target_ptr + 0) = static_cast<double>(obj->red);
  *(target_ptr + 1) = static_cast<double>(obj->green);
  *(target_ptr + 2) = static_cast<double>(obj->blue);
  *(target_ptr + 3) = static_cast<double>(obj->alpha);

  return serial_data;
}

MARSHAL_LOAD_DEF(Color) {
  const size_t size = data.size();
  if (size < sizeof(double) * 4)
    throw Exception("invalid data length, size: {}", size);

  const double* ptr = reinterpret_cast<const double*>(data.data());
  const float red = static_cast<float>(*(ptr + 0));
  const float green = static_cast<float>(*(ptr + 1));
  const float blue = static_cast<float>(*(ptr + 2));
  const float alpha = static_cast<float>(*(ptr + 3));

  return MakeRefCounted<Color>(red, green, blue, alpha);
}

MARSHAL_DUMP_DEF(Tone) {
  std::string serial_data(sizeof(double) * 4, 0);

  double* target_ptr = reinterpret_cast<double*>(serial_data.data());
  *(target_ptr + 0) = static_cast<double>(obj->red);
  *(target_ptr + 1) = static_cast<double>(obj->green);
  *(target_ptr + 2) = static_cast<double>(obj->blue);
  *(target_ptr + 3) = static_cast<double>(obj->gray);

  return serial_data;
}

MARSHAL_LOAD_DEF(Tone) {
  const size_t size = data.size();
  if (size < sizeof(double) * 4)
    throw Exception("invalid data length, size: {}", size);

  const double* ptr = reinterpret_cast<const double*>(data.data());
  const float red = static_cast<float>(*(ptr + 0));
  const float green = static_cast<float>(*(ptr + 1));
  const float blue = static_cast<float>(*(ptr + 2));
  const float gray = static_cast<float>(*(ptr + 3));

  return MakeRefCounted<Tone>(red, green, blue, gray);
}

}  // namespace rgssx
