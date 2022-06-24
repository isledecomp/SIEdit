#include "chunk.h"

#include <algorithm>
#include <iostream>

#include "data/generic.h"
#include "data/mxst.h"
#include "data/riff.h"

Chunk::Chunk()
{
  id_ = 0;

  data_ = NULL;

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

  return Read(file);
}

bool Chunk::Read(std::ifstream &f)
{
  u32 size;

  // Read ID and size, which every chunk starts with
  f.read((char *) &id_, sizeof(id_));
  f.read((char *) &size, sizeof(size));

  //std::string s(reinterpret_cast<const char*>(&id_), 4);

  // Store end of this chunk
  u32 pos = f.tellg();
  u32 end = pos + size;

  // Read custom data from this chunk
  Clear();
  data_ = CreateDataFromID(id_);
  data_->Read(f, size);

  // Assume any remaining data is this chunk's children
  while (f.good() && f.tellg() < end) {
    Chunk *child = new Chunk();
    child->Read(f);
    this->AppendChild(child);
  }

  if (f.tellg()%2 == 1) {
    f.seekg(1, std::ios::cur);
  }

  return true;
}

void Chunk::Clear()
{
  // Delete data
  if (data_) {
    delete data_;
    data_ = NULL;
  }

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

Data *Chunk::CreateDataFromID(u32 id)
{
  switch (static_cast<Type>(id)) {
  case RIFF:
    return new RIFFData();
  case LIST:
    return new RIFFData();
  case MxSt:
    return new MxStData();
  }

  return new GenericData();
}

const char *Chunk::GetTypeDescription(Type type)
{
  switch (type) {
  case RIFF:
    return "Resource Interchange File Format";
  case LIST:
    return "List of sub-elements";
  case MxSt:
    return "MxStreamer";
  }

  return "Unknown";
}


