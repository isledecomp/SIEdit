#include "chunk.h"

#include <algorithm>
#include <cstring>
#include <iostream>

#include "sitypes.h"

namespace si {

Chunk::Chunk(uint32_t id) :
  id_(id),
  offset_(0)
{
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

bool Chunk::Write(std::ofstream &f, uint32_t &version, uint32_t &alignment) const
{
  RIFF *writer = GetReaderFromType(type());

  if (alignment != 0) {
    // Determine if we have enough space left to write this chunk without the alignment
    size_t expected_write = 8;
    for (DataMap::const_iterator it=data_.begin(); it!=data_.end(); it++) {
      expected_write += it->second.size();
    }

    size_t start_chunk = f.tellp()/alignment;
    size_t end_chunk = (size_t(f.tellp())+expected_write)/alignment;
    if (start_chunk != end_chunk) {
      // This chunk is going to cross a boundary. We could write padding or split the chunk.
      // I'm not exactly sure how Weaver decides which, but I suppose it doesn't matter.
      size_t diff = (end_chunk * alignment) - f.tellp();
      pad_::WriteArbitraryPadding(f, diff - 8);
      /*if (id_ != Chunk::TYPE_MxCh || diff < 0x200) {
        // Make padding
        pad_::WriteArbitraryPadding(f, diff - 8);
      } else {
        // Attempt to split chunk
      }*/
    }
  }

  // Write 4-byte ID
  f.write((const char *) &id_, sizeof(id_));

  // Write placeholder for size and store position so we can come back later
  uint32_t chunk_size = 0;
  std::ios::pos_type size_pos = f.tellp();
  f.write((const char *) &chunk_size, sizeof(chunk_size));

  if (writer) {
    writer->Write(f, data_, version);

    if (type() == TYPE_MxHd) {
      version = data_.at("Version");
      alignment = data_.at("BufferSize");
    }

    delete writer;
  }

  for (Children::const_iterator it=GetChildren().begin(); it!=GetChildren().end(); it++) {
    static_cast<Chunk*>(*it)->Write(f, version, alignment);
  }

  // Backtrack and write chunk size
  chunk_size = uint32_t(f.tellp()) - (uint32_t(size_pos) + sizeof(uint32_t));
  f.seekp(size_pos);
  f.write((const char *) &chunk_size, sizeof(chunk_size));
  f.seekp(0, std::ios::end);

  // Byte align to 2
  if (chunk_size%2 == 1) {
    const char nothing = 0;
    f.write(&nothing, 1);
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
  case TYPE_MxDa:
    break;
  }

  return NULL;
}

void Chunk::Clear()
{
  // Delete data
  data_.clear();

  // Delete children
  DeleteChildren();
}

bool Chunk::Write(const std::string &f)
{
  return Write(f.c_str());
}

bool Chunk::Write(const char *f)
{
  std::ofstream file(f, std::ios::out | std::ios::binary);
  if (!file.is_open() || !file.good()) {
    return false;
  }

  uint32_t version = 0, alignment = 0;

  return Write(file, version, alignment);
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
  case TYPE_MxDa:
    return "Data";
  }

  return "Unknown";
}

Chunk *Chunk::FindChildWithType(Type type) const
{
  for (Children::const_iterator it=GetChildren().begin(); it!=GetChildren().end(); it++) {
    Chunk *chunk = static_cast<Chunk*>(*it);
    if (chunk->type() == type) {
      return chunk;
    } else if (Chunk *grandchild = chunk->FindChildWithType(type)) {
      return grandchild;
    }
  }

  return NULL;
}

Chunk *Chunk::FindChildWithOffset(uint32_t offset) const
{
  for (Children::const_iterator it=GetChildren().begin(); it!=GetChildren().end(); it++) {
    Chunk *chunk = static_cast<Chunk*>(*it);
    if (chunk->offset() == offset) {
      return chunk;
    } else if (Chunk *grandchild = chunk->FindChildWithOffset(offset)) {
      return grandchild;
    }
  }

  return NULL;
}

}
