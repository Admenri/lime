#pragma once

#include <vector>

#include "refptr.h"

namespace rgssx {

class Table : public RefCounted<Table> {
 public:
  Table(int xsize, int ysize = 1, int zsize = 1);

  void Resize(int xsize, int ysize = 1, int zsize = 1);
  int XSize();
  int YSize();
  int ZSize();

  int16_t Get(int x, int y = 0, int z = 0);
  void Set(int16_t value, int x, int y = 0, int z = 0);

 private:
  int xsize_ = 0, ysize_ = 0, zsize_ = 0;
  std::vector<int16_t> data_;
};

}  // namespace rgssx
