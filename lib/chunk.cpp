#include "chunk.h"

#include <algorithm>
#include <cstring>
#include <iostream>

#include "data/mxch.h"
#include "data/mxhd.h"
#include "data/mxob.h"
#include "data/mxof.h"
#include "data/mxst.h"
#include "data/riff.h"

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

template <typename T>
void CreateAndReadData(std::ifstream &f, bytearray &data, u32 size)
{
  size_t read_sz = std::min(size_t(size), sizeof(T));
  data.resize(read_sz);
  f.read(data.data(), read_sz);
}

void ReadNullTerminatedString(std::ifstream &f, char *str)
{
  std::string s;

  while (true) {
    f.read(str, 1);
    if (*str == 0) {
      break;
    } else {
      str++;
    }
  }
}

void Read_List(std::ifstream &f, LIST *ls)
{
  f.read((char *) &ls->dwID, sizeof(ls->dwID));

  if (ls->dwID == Chunk::TYPE_MxCh) {
    // MxCh type lists contain an element count
    f.read((char *) &ls->dwCount, sizeof(ls->dwCount));

    // TEMP FOR 1.2 REMOVE
    //f.read((char *) &ls->dwCount, sizeof(ls->dwCount));
  } else {
    ls->dwCount = 0;
  }
}

void Read_MxOb(std::ifstream &f, Chunk *c, MxOb *ob)
{
  f.read((char *) &ob->wType, sizeof(ob->wType));

  ReadNullTerminatedString(f, ob->szPresenter);

  f.read((char *) &ob->dwUnknown1, sizeof(ob->dwUnknown1));

  ReadNullTerminatedString(f, ob->szName);

  f.read((char *) &ob->dwObjectID, sizeof(ob->dwObjectID));
  f.read((char *) &ob->dwUnknown3, sizeof(ob->dwUnknown3));
  f.read((char *) &ob->dwUnknown4, sizeof(ob->dwUnknown4));
  f.read((char *) &ob->dwUnknown5, sizeof(ob->dwUnknown5));
  f.read((char *) &ob->dwUnknown6, sizeof(ob->dwUnknown6));
  f.read((char *) &ob->dwUnknown7, sizeof(ob->dwUnknown7));
  f.read((char *) &ob->dwUnknown8, sizeof(ob->dwUnknown8));
  f.read((char *) &ob->dwUnknown9, sizeof(ob->dwUnknown9));
  f.read((char *) &ob->dwUnknown10, sizeof(ob->dwUnknown10));
  f.read((char *) &ob->dwUnknown11, sizeof(ob->dwUnknown11));
  f.read((char *) &ob->dwUnknown12, sizeof(ob->dwUnknown12));
  f.read((char *) &ob->dwUnknown13, sizeof(ob->dwUnknown13));
  f.read((char *) &ob->dwUnknown14, sizeof(ob->dwUnknown14));
  f.read((char *) &ob->dwUnknown15, sizeof(ob->dwUnknown15));
  f.read((char *) &ob->dwUnknown16, sizeof(ob->dwUnknown16));
  f.read((char *) &ob->dwUnknown17, sizeof(ob->dwUnknown17));
  f.read((char *) &ob->fUnknown18, sizeof(ob->fUnknown18));
  f.read((char *) &ob->dwUnknown19, sizeof(ob->dwUnknown19));
  f.read((char *) &ob->dwUnknown20, sizeof(ob->dwUnknown20));
  f.read((char *) &ob->dwUnknown21, sizeof(ob->dwUnknown21));
  f.read((char *) &ob->fUnknown22, sizeof(ob->fUnknown22));
  f.read((char *) &ob->dwUnknown23, sizeof(ob->dwUnknown23));
  f.read((char *) &ob->dwUnknown24, sizeof(ob->dwUnknown24));
  f.read((char *) &ob->wCreationStringLength, sizeof(ob->wCreationStringLength));

  if (ob->wCreationStringLength > 0) {
    f.read(ob->szCreationString, ob->wCreationStringLength);
  }

  switch (static_cast<MxOb::Type>(ob->wType)) {
  case MxOb::Presenter:
  case MxOb::World:
    break;
  case MxOb::OBJ:
  case MxOb::BMP:
  case MxOb::SMK:
  case MxOb::WAV:
  case MxOb::Event:
    ReadNullTerminatedString(f, ob->szFilename);

    if (ob->wType != MxOb::World) {
      f.read((char *) &ob->dwUnknown26, sizeof(ob->dwUnknown26));
      f.read((char *) &ob->dwUnknown27, sizeof(ob->dwUnknown27));
      f.read((char *) &ob->dwUnknown28, sizeof(ob->dwUnknown28));

      f.read((char *) &ob->dwID, sizeof(ob->dwID));

      f.read((char *) &ob->dwUnknown29, sizeof(ob->dwUnknown29));
      f.read((char *) &ob->dwUnknown30, sizeof(ob->dwUnknown30));

      if (ob->wType == MxOb::WAV) {
        f.read((char *) &ob->dwUnknown31, sizeof(ob->dwUnknown31));
      }
    }
    break;
  case MxOb::TYPE_COUNT:
    break;
  }
}

bool Chunk::Read(std::ifstream &f, u32 &version, u32 &alignment)
{
  u32 size;

  offset_ = f.tellg();

  // Read ID and size, which every chunk starts with
  f.read((char *) &id_, sizeof(id_));
  f.read((char *) &size, sizeof(size));

  // Store end of this chunk
  u32 pos = f.tellg();
  u32 end = pos + size;

  // Read custom data from this chunk
  Clear();

  switch (type()) {
  case TYPE_RIFF:
    CreateAndReadData<RIFF>(f, data_, size);
    break;
  case TYPE_LIST:
    data_.resize(sizeof(LIST));
    Read_List(f, data_.cast<LIST>());
    break;
  case TYPE_MxSt:
    // MxSt is 0 bytes in size, and an empty struct in C++ is one byte which throws things off,
    // so we just create nothing here
    size = 0;
    break;
  case TYPE_MxHd:
  {
    CreateAndReadData<MxHd>(f, data_, size);

    MxHd *mxhd = data_.cast<MxHd>();
    alignment = mxhd->dwBufferSize;
    version = mxhd->dwVersion;
    break;
  }
  case TYPE_MxCh:
    CreateAndReadData<MxCh>(f, data_, size);
    exdata_.resize(size - sizeof(MxCh));
    break;
  case TYPE_MxOf:
    CreateAndReadData<MxOf>(f, data_, size);
    exdata_.resize(size - sizeof(MxOf));
    break;
  case TYPE_pad_:
    break;
  case TYPE_MxOb:
  {
    data_.resize(sizeof(MxOb));
    Read_MxOb(f, this, data_.cast<MxOb>());
    break;
  }
  }

  // Handle unknown data
  if (data_.empty() && size > 0) {
    data_.resize(size);
    f.read((char*)data_.data(), size);
  }

  // Handle extended data
  if (!exdata_.empty()) {
    f.read((char*) exdata_.data(), exdata_.size());
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

  // Delete exdata
  exdata_.clear();

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
