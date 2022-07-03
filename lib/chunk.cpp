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

  u32 alignment = 0, version = 0;

  return Read(file, version, alignment);
}

bool Chunk::Read(std::ifstream &f, u32 &version, u32 &alignment)
{
  u32 size;

  offset_ = u32(f.tellg());

  // Read ID and size, which every chunk starts with
  f.read((char *) &id_, sizeof(id_));
  f.read((char *) &size, sizeof(size));

  // Store end of this chunk
  u32 pos = u32(f.tellg());
  u32 end = pos + size;

  // Read custom data from this chunk
  Clear();

  switch (type()) {
  case TYPE_RIFF:
    RIFF().Read(f, data_, version, size);
    break;
  case TYPE_MxHd:
    MxHd().Read(f, data_, version, size);
    version = data_["Version"];
    alignment = data_["BufferSize"];
    break;
  case TYPE_LIST:
    LIST().Read(f, data_, version, size);
    break;
  case TYPE_MxSt:
    MxSt().Read(f, data_, version, size);
    break;
  case TYPE_MxCh:
    MxCh().Read(f, data_, version, size);
    break;
  case TYPE_MxOf:
    MxOf().Read(f, data_, version, size);
    break;
  case TYPE_pad_:
    pad_().Read(f, data_, version, size);
    break;
  case TYPE_MxOb:
    MxOb().Read(f, data_, version, size);
    break;
  }

  // Assume any remaining data is this chunk's children
  while (f.good() && (size_t(f.tellg()) + 4) < end) {
    // Check alignment, if there's not enough room to for another segment, skip ahead
    if (alignment > 0) {
      u32 offset_in_buffer = f.tellg()%alignment;
      if (offset_in_buffer + sizeof(u32)*2 > alignment) {
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

}
