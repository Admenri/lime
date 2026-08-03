#pragma once

#include <vector>

#include "src/define.h"
#include "src/refptr.h"

namespace rgssx {

class Table : public RefCounted<Table> {
 public:
  Table(int xsize, int ysize = 1, int zsize = 1);

  /*-export.begin-*/
  MARSHAL_DUMP(Table);
  MARSHAL_LOAD(Table);

  void Resize(int xsize, int ysize = 1, int zsize = 1);
  int XSize();
  int YSize();
  int ZSize();

  /* [] */ int16_t Get(int x, int y = 0, int z = 0);
  /* []= */ void Set(int16_t value, int x, int y = 0, int z = 0);
  /*-export.end-*/

 private:
  int xsize_ = 0, ysize_ = 0, zsize_ = 0;
  std::vector<int16_t> data_;
};

}  // namespace rgssx
