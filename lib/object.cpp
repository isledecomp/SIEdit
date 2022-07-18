#include "object.h"

#include <iostream>

#include "util.h"

namespace si {

Object::Object()
{
  type_ = MxOb::Null;
  id_ = 0;
}

/*bool Object::Read(std::ifstream &is)
{


  if (chunk->HasChildren()) {
    Chunk *child = static_cast<Chunk*>(chunk->GetChildAt(0));
    if (child->id() == Chunk::TYPE_LIST) {
      for (Children::const_iterator it=child->GetChildren().begin(); it!=child->GetChildren().end(); it++) {
        Object *o = new Object();
        if (!o->Parse(static_cast<Chunk*>(*it))) {
          return false;
        }
        AppendChild(o);
      }
    }
  }

  return true;
}

void Object::Write(std::ofstream &os) const
{
  WriteU16(os, type_);
  WriteString(os, presenter_);
  WriteU32(os, unknown1_);
  WriteString(os, name_);
  WriteU32(os, id_);
  WriteU32(os, flags_);
  WriteU32(os, unknown4_);
  WriteU32(os, duration_);
  WriteU32(os, loops_);
  WriteVector3(os, position_);
  WriteVector3(os, direction_);
  WriteVector3(os, up_);
  WriteU16(os, extra_.size());
  WriteBytes(os, extra_);
  WriteString(os, filename_);
  WriteU32(os, unknown26_);
  WriteU32(os, unknown27_);
  WriteU32(os, unknown28_);
  WriteU32(os, filetype_);
  WriteU32(os, unknown29_);
  WriteU32(os, unknown30_);
  WriteU32(os, unknown31_);

  if (HasChildren()) {
    Chunk *list = new Chunk(Chunk::TYPE_LIST);
    list->data("Format") = Chunk::TYPE_MxCh;
    list->data("Count") = list->GetChildCount();
    chunk->AppendChild(list);

    for (Children::const_iterator it=GetChildren().begin(); it!=GetChildren().end(); it++) {
      Object *child = static_cast<Object*>(*it);
      list->AppendChild(child->Export());
    }
  }

  return chunk;
}*/

bytearray Object::GetNormalizedData() const
{
  return ToPackedData(filetype(), data_);
}

void Object::SetNormalizedData(const bytearray &d)
{
  SetChunkedData(ToChunkedData(filetype(), d));
}

bytearray Object::ToPackedData(MxOb::FileType filetype, const ChunkedData &chunks)
{
  bytearray data;

  switch (filetype) {
  case MxOb::WAV:
  {
    // Make space for WAVE header
    data.resize(0x2C);

    // Merge all chunks after the first one
    for (size_t i=1; i<chunks.size(); i++) {
      data.append(chunks[i]);
    }

    // Copy boilerplate bytes for header
    uint32_t *header = reinterpret_cast<uint32_t *>(data.data());
    header[0] = SI::RIFF;     // "RIFF"
    header[1] = data.size() - 8;     // Size of total file
    header[2] = 0x45564157;           // "WAVE"
    header[3] = 0x20746D66;           // "fmt "
    header[4] = 16;                   // Size of fmt chunk
    header[9] = 0x61746164;           // "data"
    header[10] = data.size() - 0x2C; // Size of data chunk

    // Copy fmt header from chunk 1
    memcpy(&header[5], chunks[0].data(), 16);
    break;
  }
  case MxOb::STL:
  {
    // Make space for BMP header
    data.resize(14);

    // Merge all chunks after the first one
    for (size_t i=0; i<chunks.size(); i++) {
      data.append(chunks[i]);
    }

    // Set BM identifier
    *(uint16_t *)(data.data()) = 0x4D42;

    // Set file size
    *(uint32_t*)(data.data()+2) = data.size();

    // Set reserved bytes
    *(uint32_t*)(data.data()+6) = 0;

    // Set offset
    *(uint32_t*)(data.data()+10) = chunks.at(0).size() + 14;
    break;
  }
  case MxOb::FLC:
  {
    // First chunk is a complete FLIC header, so add it as-is
    data.append(chunks[0]);

    // Subsequent chunks are FLIC frames with an additional 20 byte header that needs to be stripped
    const int CUSTOM_HEADER_SZ = 20;
    for (size_t i=1; i<chunks.size(); i++) {
      data.append(chunks.at(i).data() + CUSTOM_HEADER_SZ, chunks.at(i).size() - CUSTOM_HEADER_SZ);
    }
    break;
  }
  case MxOb::SMK:
  case MxOb::OBJ:
  {
    // Simply merge
    for (size_t i=0; i<chunks.size(); i++) {
      data.append(chunks[i]);
    }
    break;
  }
  default:
    std::cout << "Didn't know how to extract type '" << std::string((const char *)&filetype, sizeof(filetype)) << "', merging..." << std::endl;
    for (size_t i=0; i<chunks.size(); i++) {
      data.append(chunks[i]);
    }
    break;
  }

  return data;
}

Object::ChunkedData Object::ToChunkedData(MxOb::FileType filetype, const bytearray &chunks)
{
  // FIXME: STUB
  return ChunkedData();
}

const bytearray &Object::GetFileHeader() const
{
  return data_.at(0);
}

bytearray Object::GetFileBody() const
{
  bytearray b;

  for (size_t i=1; i<data_.size(); i++) {
    b.append(data_.at(i));
  }

  return b;
}

size_t Object::GetFileBodySize() const
{
  size_t s = 0;

  for (size_t i=1; i<data_.size(); i++) {
    s += data_.at(i).size();
  }

  return s;
}

Object *Object::FindSubObjectWithID(uint32_t id)
{
  if (this->id() == id) {
    return this;
  }

  for (Children::const_iterator it=GetChildren().begin(); it!=GetChildren().end(); it++) {
    if (Object *o = static_cast<Object*>(*it)->FindSubObjectWithID(id)) {
      return o;
    }
  }

  return NULL;
}

}
