#include "drawable.h"

namespace rgssx {

// ─── Drawable ────────────────────────────────────────────────────────────────

Drawable::Drawable() : prev_(this), next_(this) {}

Drawable::~Drawable() {
  RemoveFromList();
}

ATTR_DEF(bool, Visible, Drawable) {
  if (value.has_value()) {
    visible_ = *value;
    return std::nullopt;
  } else {
    return visible_;
  }
}

ATTR_DEF(int, Z, Drawable) {
  if (value.has_value()) {
    if (*value != z_.value) {
      auto old_z = z_;
      z_.value = *value;
      Resort(old_z);
    }
    return std::nullopt;
  } else {
    return z_.value;
  }
}

void Drawable::Resort(ZValue old) {
  if (parent_ != nullptr) {
    if (z_ < old) {
      BubbleLeft();
    } else {
      BubbleRight();
    }
  }
}

void Drawable::SetParent(DrawableSet* parent) {
  if (parent_ != parent) {
    RemoveFromList();
    parent_ = parent;
    if (parent_) {
      // Insert into new list at correct z-order position:
      // find the first node whose z > this->z_, then insert before it.
      Drawable* sentinel = &parent_->root_;
      Drawable* pos = sentinel->next_;
      while (pos != sentinel && pos->z_ <= z_)
        pos = pos->next_;
      InsertAfter(pos->prev_);  // insert before 'pos'
    }
  }
}

void Drawable::InsertAfter(Drawable* node) {
  next_ = node->next_;
  prev_ = node;
  node->next_->prev_ = this;
  node->next_ = this;
}

void Drawable::RemoveFromList() {
  prev_->next_ = next_;
  next_->prev_ = prev_;
  prev_ = this;
  next_ = this;
}

void Drawable::BubbleLeft() {
  // z decreased: move left past any node with higher z
  while (prev_ != &parent_->root_ && prev_->z_ > z_) {
    Drawable* target = prev_;
    RemoveFromList();
    // re-insert before target
    prev_ = target->prev_;
    next_ = target;
    target->prev_->next_ = this;
    target->prev_ = this;
  }
}

void Drawable::BubbleRight() {
  // z increased: move right past any node with lower z
  while (next_ != &parent_->root_ && next_->z_ < z_) {
    Drawable* target = next_;
    RemoveFromList();
    // re-insert after target
    next_ = target->next_;
    prev_ = target;
    target->next_->prev_ = this;
    target->next_ = this;
  }
}

// ─── DrawableSet
// ─────────────────────────────────────────────────────────────

DrawableSet::DrawableSet() {
  root_.prev_ = &root_;
  root_.next_ = &root_;
}

DrawableSet::~DrawableSet() {
  // Detach all remaining drawables so they don't hold dangling parent
  // pointers.
  while (root_.next_ != &root_) {
    root_.next_->parent_ = nullptr;
    root_.next_->RemoveFromList();
  }
}

void DrawableSet::DispatchPrepare() {
  for (Drawable* node = root_.next_; node != &root_; node = node->next_) {
    if (node->visible_) {
      node->Prepare();
    }
  }
}

void DrawableSet::DispatchDraw(DrawParam param) {
  for (Drawable* node = root_.next_; node != &root_; node = node->next_) {
    if (node->visible_) {
      node->Draw(param);
    }
  }
}

}  // namespace rgssx
