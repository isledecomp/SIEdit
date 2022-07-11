#include "object.h"

namespace si {

Object::Object()
{

}

bool Object::Parse(Chunk *chunk)
{
  type_ = static_cast<MxOb::Type>(chunk->data("Type").toU16());
  presenter_ = chunk->data("Presenter").toString();
  unknown1_ = chunk->data("Unknown1");
  name_ = chunk->data("Name").toString();
  id_ = chunk->data("ID");
  flags_ = chunk->data("Flags");
  unknown4_ = chunk->data("Unknown4");
  duration_ = chunk->data("Duration");
  loops_ = chunk->data("Loops");
  position_ = chunk->data("Position");
  direction_ = chunk->data("Direction");
  up_ = chunk->data("Up");
  extra_ = chunk->data("ExtraData");
  filename_ = chunk->data("FileName").toString();
  unknown26_ = chunk->data("Unknown26");
  unknown27_ = chunk->data("Unknown27");
  unknown28_ = chunk->data("Unknown28");
  filetype_ = static_cast<MxOb::FileType>(chunk->data("FileType").toU32());
  unknown29_ = chunk->data("Unknown29");
  unknown30_ = chunk->data("Unknown30");
  unknown31_ = chunk->data("Unknown31");

  if (chunk->HasChildren()) {
    Chunk *child = static_cast<Chunk*>(chunk->GetChildAt(0));
    if (child->id() == Chunk::TYPE_LIST) {
      for (Core *entry : child->GetChildren()) {
        Object *o = new Object();
        if (!o->Parse(static_cast<Chunk*>(entry))) {
          return false;
        }
        AppendChild(o);
      }
    }
  }

  return true;
}

void Object::ProcessData(const std::vector<bytearray> &chunks)
{
  switch (filetype_) {
  case MxOb::WAV:
  {
    // Make space for WAVE header
    data_.resize(0x2C);

    // Merge all chunks after the first one
    for (size_t i=1; i<chunks.size(); i++) {
      data_.append(chunks[i]);
    }

    // Copy boilerplate bytes for header
    uint32_t *header = reinterpret_cast<uint32_t *>(data_.data());
    header[0] = Chunk::TYPE_RIFF;     // "RIFF"
    header[1] = data_.size() - 8;     // Size of total file
    header[2] = 0x45564157;           // "WAVE"
    header[3] = 0x20746D66;           // "fmt "
    header[4] = 16;                   // Size of fmt chunk
    header[9] = 0x61746164;           // "data"
    header[10] = data_.size() - 0x2C; // Size of data chunk

    // Copy fmt header from chunk 1
    memcpy(&header[5], chunks[0].data(), 16);
    break;
  }
  case MxOb::STL:
  {
    // Make space for BMP header
    data_.resize(14);

    // Merge all chunks after the first one
    for (size_t i=0; i<chunks.size(); i++) {
      data_.append(chunks[i]);
    }

    // Set BM identifier
    *(uint16_t *)(data_.data()) = 0x4D42;

    // Set file size
    *(uint32_t*)(data_.data()+2) = data_.size();

    // Set reserved bytes
    *(uint32_t*)(data_.data()+6) = 0;

    // Set offset
    *(uint32_t*)(data_.data()+10) = chunks.at(0).size() + 14;
    break;
  }
  }
}

Object *Object::FindSubObjectWithID(uint32_t id)
{
  if (this->id() == id) {
    return this;
  }

  for (Core *child : GetChildren()) {
    if (Object *o = static_cast<Object*>(child)->FindSubObjectWithID(id)) {
      return o;
    }
  }

  return NULL;
}

}
