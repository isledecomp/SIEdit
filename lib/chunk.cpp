#include "chunk.h"

#include <algorithm>
#include <cstring>
#include <iostream>

#include "sitypes.h"

namespace si {

Chunk::Chunk()
{
  id_ = 0;

  parent_ = NULL;
}

Chunk::~Chunk()
{
  Clear();
}

bool Chunk::Read(const std::string &f)
{
  return Read(f.c_str());
}

bool Chunk::Read(const char *f)
{
  std::ifstream file(f, std::ios::in | std::ios::binary);
  if (!file.is_open() || !file.good()) {
    return false;
  }

  uint32_t alignment = 0, version = 0;

  return Read(file, version, alignment);
}

bool Chunk::Read(std::ifstream &f, uint32_t &version, uint32_t &alignment)
{
  uint32_t size;

  offset_ = uint32_t(f.tellg());

  // Read ID and size, which every chunk starts with
  f.read((char *) &id_, sizeof(id_));
  f.read((char *) &size, sizeof(size));

  // Store end of this chunk
  uint32_t pos = uint32_t(f.tellg());
  uint32_t end = pos + size;

  // Read custom data from this chunk
  Clear();

  if (RIFF *reader = GetReaderFromType(type())) {
    reader->Read(f, data_, version, size);

    if (type() == TYPE_MxHd) {
      version = data_["Version"];
      alignment = data_["BufferSize"];
    }

    delete reader;
  }

  // Assume any remaining data is this chunk's children
  while (f.good() && (size_t(f.tellg()) + 4) < end) {
    // Check alignment, if there's not enough room to for another segment, skip ahead
    if (alignment > 0) {
      uint32_t offset_in_buffer = f.tellg()%alignment;
      if (offset_in_buffer + sizeof(uint32_t)*2 > alignment) {
        f.seekg(alignment-offset_in_buffer, std::ios::cur);
      }
    }

    Chunk *child = new Chunk();
    child->Read(f, version, alignment);
    this->AppendChild(child);
  }

  if (f.tellg() < end) {
    f.seekg(end, std::ios::beg);
  }

  if (size%2 == 1) {
    f.seekg(1, std::ios::cur);
  }

  return true;
}

RIFF *Chunk::GetReaderFromType(Type type)
{
  switch (type) {
  case TYPE_RIFF:
    return new RIFF();
  case TYPE_MxHd:
    return new MxHd();
  case TYPE_LIST:
    return new LIST();
  case TYPE_MxSt:
    return new MxSt();
  case TYPE_MxCh:
    return new MxCh();
  case TYPE_MxOf:
    return new MxOf();
  case TYPE_pad_:
    return new pad_();
  case TYPE_MxOb:
    return new MxOb();
  }

  return NULL;
}

void Chunk::Clear()
{
  // Delete data
  data_.clear();

  // Delete children
  for (Children::iterator it = children_.begin(); it != children_.end(); it++) {
    delete (*it);
  }
  children_.clear();
}

void Chunk::AppendChild(Chunk *chunk)
{
  // If this chunk has another parent, remove it from that parent
  if (chunk->parent_) {
    chunk->parent_->RemoveChild(chunk);
  }

  // Append it to this chunk
  chunk->parent_ = this;
  children_.push_back(chunk);
}

bool Chunk::RemoveChild(Chunk *chunk)
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

size_t Chunk::IndexOfChild(Chunk *chunk)
{
  return std::find(children_.begin(), children_.end(), chunk) - children_.begin();
}

void Chunk::InsertChild(size_t index, Chunk *chunk)
{
  // If this chunk has another parent, remove it from that parent
  if (chunk->parent_) {
    chunk->parent_->RemoveChild(chunk);
  }

  // Insert at position
  chunk->parent_ = this;
  children_.insert(children_.begin() + index, chunk);
}

const char *Chunk::GetTypeDescription(Type type)
{
  switch (type) {
  case TYPE_RIFF:
    return "Resource Interchange File Format";
  case TYPE_LIST:
    return "List of sub-elements";
  case TYPE_MxSt:
    return "Stream";
  case TYPE_MxHd:
    return "Interleaf Header";
  case TYPE_MxCh:
    return "Data Chunk";
  case TYPE_MxOf:
    return "Offset Table";
  case TYPE_pad_:
    return "Padding";
  case TYPE_MxOb:
    return "Streamable Object";
  }

  return "Unknown";
}

Chunk *Chunk::FindChildWithType(Type type) const
{
  for (Chunk *child : children_) {
    if (child->type() == type) {
      return child;
    } else if (Chunk *grandchild = child->FindChildWithType(type)) {
      return grandchild;
    }
  }

  return NULL;
}

Chunk *Chunk::FindChildWithOffset(u32 offset) const
{
  for (Chunk *child : children_) {
    if (child->offset() == offset) {
      return child;
    } else if (Chunk *grandchild = child->FindChildWithOffset(offset)) {
      return grandchild;
    }
  }

  return NULL;
}

}
