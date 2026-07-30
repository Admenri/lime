#include "table.h"

namespace rgssx {

Table::Table(int xsize, int ysize, int zsize) {
  Resize(xsize, ysize, zsize);
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
