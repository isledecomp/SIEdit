#include "core.h"

#include <algorithm>

namespace si {

Core::Core()
{
  parent_ = NULL;
}

Core::~Core()
{
  // Remove from parent if applicable
  if (parent_) {
    parent_->RemoveChild(this);
  }

  DeleteChildren();
}

bool Core::FindParent(Core *p) const
{
  Core *parent = this->parent_;
  while (parent != NULL) {
    if (parent == p) {
      return true;
    } else {
      parent = parent->parent_;
    }
  }

  return false;
}

void Core::AppendChild(Core *chunk)
{
  InsertChild(children_.size(), chunk);
}

bool Core::RemoveChild(Core *chunk)
{
  // If this chunk's parent is not this, return
  if (chunk->parent_ != this) {
    return false;
  }

  // Find chunk in children, if doesn't exist, return false
  Children::iterator it = std::find(children_.begin(), children_.end(), chunk);
  if (it == children_.end()) {
    return false;
  }

  chunk->parent_ = NULL;
  children_.erase(it);
  return true;
}

void Core::DeleteChildren()
{
  // Delete children
  Children copy = children_;
  for (Children::iterator it = copy.begin(); it != copy.end(); it++) {
    delete (*it);
  }
}

size_t Core::IndexOfChild(Core *chunk) const
{
  return std::find(children_.begin(), children_.end(), chunk) - children_.begin();
}

void Core::InsertChild(size_t index, Core *chunk)
{
  if (chunk == this || FindParent(chunk)) {
    return;
  }

  // If this chunk has another parent, remove it from that parent
  if (chunk->parent_) {
    chunk->parent_->RemoveChild(chunk);
  }

  // Insert at position
  chunk->parent_ = this;
  children_.insert(children_.begin() + index, chunk);
}

}
