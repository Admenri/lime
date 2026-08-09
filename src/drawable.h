#pragma once

#include "src/common.h"
#include "src/raywarp.h"
#include "src/utility.h"

namespace lime {

class DrawableSet;

struct DrawParam {
  int ox = 0, oy = 0;
  raylib::Rectangle scissor;
  raylib::RenderTexture2D target;
};

struct ZValue {
  int value = 0;
  int sorting = 0;
  double timestamp = 0;

  ZValue() : timestamp(raylib::GetTime()) {}
  ZValue(int v) : value(v), timestamp(raylib::GetTime()) {}
  ZValue(int v, int s) : value(v), sorting(s), timestamp(raylib::GetTime()) {}

  bool operator<(const ZValue& other) const {
    if (value != other.value)
      return value < other.value;
    if (sorting != other.sorting)
      return sorting < other.sorting;
    return timestamp < other.timestamp;
  }

  bool operator>(const ZValue& other) const { return other < *this; }
  bool operator<=(const ZValue& other) const { return !(*this > other); }
  bool operator>=(const ZValue& other) const { return !(*this < other); }
};

class Drawable {
 public:
  Drawable();
  Drawable(const ZValue& order);
  virtual ~Drawable();

  /*-export.begin-*/
  virtual ATTR(bool, Visible);
  virtual ATTR(int, Z);
  /*-export.end-*/

 public:
  virtual void Draw(DrawParam param) {}

 protected:
  ZValue& order() { return z_; }

  void Resort(ZValue old);
  void SetParent(DrawableSet* parent);
  void InsertAfter(Drawable* node);
  void RemoveFromList();

 private:
  friend class DrawableSet;

  void BubbleLeft();
  void BubbleRight();

  Drawable* prev_ = nullptr;
  Drawable* next_ = nullptr;

  DrawableSet* parent_ = nullptr;
  bool visible_ = true;
  ZValue z_ = {};
};

class DrawableSet {
 public:
  DrawableSet();
  ~DrawableSet();

  void DispatchDraw(DrawParam param);

 private:
  friend class Drawable;
  Drawable root_;  // sentinel node for doubly-linked list
};

}  // namespace lime
