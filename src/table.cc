#include "src/table.h"

#include "src/common.h"

namespace rgssx {

Table::Table(int xsize, int ysize, int zsize) {
  Resize(xsize, ysize, zsize);
}

MARSHAL_DUMP_DEF(Table) {
  uint32_t dim = 0;
  if (obj->xsize_ >= 1)
    dim++;
  if (obj->ysize_ > 1)
    dim++;
  if (obj->zsize_ > 1)
    dim++;

  uint32_t data_size = obj->xsize_ * obj->ysize_ * obj->zsize_;
  std::string serial_data(sizeof(int32_t) * 5 + data_size * sizeof(int16_t), 0);

  uint32_t* ptr = reinterpret_cast<uint32_t*>(serial_data.data());
  *(ptr + 0) = dim;
  *(ptr + 1) = obj->xsize_;
  *(ptr + 2) = obj->ysize_;
  *(ptr + 3) = obj->zsize_;
  *(ptr + 4) = data_size;
  std::memcpy(ptr + 5, obj->data_.data(), data_size * sizeof(int16_t));

  return serial_data;
}

MARSHAL_LOAD_DEF(Table) {
  const uint32_t* raw_ptr = reinterpret_cast<const uint32_t*>(data.data());

  uint32_t xsize = *(raw_ptr + 1);
  uint32_t ysize = *(raw_ptr + 2);
  uint32_t zsize = *(raw_ptr + 3);
  uint32_t data_size = *(raw_ptr + 4);

  RefPtr<Table> obj = MakeRefCounted<Table>(xsize, ysize, zsize);
  if (data_size != obj->xsize_ * obj->ysize_ * obj->zsize_)
    throw Exception(Exception::RGSSError, "invalid table serialize data");

  if (data_size)
    std::memcpy(obj->data_.data(), raw_ptr + 5, data_size * sizeof(int16_t));

  return obj;
}

void Table::Resize(int xsize, int ysize, int zsize) {
  xsize_ = xsize;
  ysize_ = ysize;
  zsize_ = zsize;

  data_.resize(xsize * ysize * zsize, 0);
  data_.shrink_to_fit();
}

int Table::XSize() {
  return xsize_;
}

int Table::YSize() {
  return ysize_;
}

int Table::ZSize() {
  return zsize_;
}

int16_t Table::Get(int x, int y, int z) {
  int index = x + y * xsize_ + z * xsize_ * ysize_;
  return data_.at(index);
}

void Table::Set(int16_t value, int x, int y, int z) {
  int index = x + y * xsize_ + z * xsize_ * ysize_;
  data_.at(index) = value;
}

}  // namespace rgssx
