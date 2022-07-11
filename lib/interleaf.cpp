#include "interleaf.h"

#include <iostream>

#include "object.h"

namespace si {

Interleaf::Interleaf()
{
}

bool Interleaf::Parse(Chunk *riff)
{
  if (riff->id() != Chunk::TYPE_RIFF) {
    return false;
  }

  Chunk *hd = riff->FindChildWithType(Chunk::TYPE_MxHd);
  if (!hd) {
    return false;
  }

  version_ = hd->data("Version");
  if (version_ == 0) {
    // Unknown version
    return false;
  }

  buffer_size_ = hd->data("BufferSize");
  buffer_count_ = hd->data("BufferCount");

  Chunk *of = riff->FindChildWithType(Chunk::TYPE_MxOf);
  if (!of) {
    return false;
  }

  const Data &offset_data = of->data("Offsets");
  const uint32_t *offset_table = reinterpret_cast<const uint32_t *>(offset_data.data());
  size_t offset_count = offset_data.size() / sizeof(uint32_t);
  DeleteChildren();
  for (size_t i=0; i<offset_count; i++) {
    if (offset_table[i]) {
      Chunk *st = riff->FindChildWithOffset(offset_table[i]);
      if (!ParseStream(st)) {
        return false;
      }
    } else {
      //Object *nullobj = new Object();
      //AppendChild(nullobj);
    }
  }

  return true;
}

bool Interleaf::ParseStream(Chunk *chunk)
{
  if (chunk->type() != Chunk::TYPE_MxSt) {
    return false;
  }

  Chunk *obj_chunk = static_cast<Chunk*>(chunk->GetChildAt(0));
  if (!obj_chunk) {
    return false;
  }

  Object *obj = new Object();
  if (!obj->Parse(obj_chunk)) {
    return false;
  }

  AppendChild(obj);

  Chunk *list = static_cast<Chunk*>(chunk->GetChildAt(1));
  if (list) {
    typedef std::map<uint32_t, Object::ChunkedData> ChunkMap;
    ChunkMap data;

    for (Children::const_iterator it=list->GetChildren().begin(); it!=list->GetChildren().end(); it++) {
      Chunk *mxch = static_cast<Chunk*>(*it);
      if (mxch->id() == Chunk::TYPE_pad_) {
        // Ignore this chunk
      } else if (mxch->id() == Chunk::TYPE_MxCh) {
        uint32_t obj_id = mxch->data("Object");
        data[obj_id].push_back(mxch->data("Data"));
      }
    }

    for (ChunkMap::const_iterator it=data.begin(); it!=data.end(); it++) {
      Object *o = obj->FindSubObjectWithID(it->first);
      if (o) {
        o->SetChunkedData(it->second);
      } else {
        std::cout << "Failed to find object with ID " << it->first << std::endl;
      }
    }
  }

  return true;
}

}
