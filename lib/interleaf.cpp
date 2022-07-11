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
      Object *nullobj = new Object();
      AppendChild(nullobj);
    }
  }

  return true;
}

struct ChunkStatus {
  ChunkStatus()
  {
    index = 0;
    time = 0;
  }

  size_t index;
  uint32_t time;
};

Chunk *Interleaf::Export() const
{
  Chunk *riff = new Chunk(Chunk::TYPE_RIFF);
  riff->data("Format") = RIFF::OMNI;

  Chunk *mxhd = new Chunk(Chunk::TYPE_MxHd);
  mxhd->data("Version") = version_;
  mxhd->data("BufferSize") = buffer_size_;
  mxhd->data("BufferCount") = buffer_count_;
  riff->AppendChild(mxhd);

  Chunk *mxof = new Chunk(Chunk::TYPE_MxOf);

  // FIXME: This appears to not always be correct, sometimes an MxOf with only one entry will have
  //        a count of 3, seemingly due to embedded objects (e.g. a movie with an SMK + WAV)?
  uint32_t obj_count = this->GetChildCount();
  for (Children::const_iterator it=GetChildren().begin(); it!=GetChildren().end(); it++) {
    Object *obj = static_cast<Object*>(*it);
    obj_count += obj->GetChildCount();
  }
  mxof->data("Count") = obj_count;

  //        This however is correct.
  mxof->data("Offsets") = bytearray(this->GetChildCount() * sizeof(uint32_t));

  riff->AppendChild(mxof);

  Chunk *list = new Chunk(Chunk::TYPE_LIST);
  list->data("Format") = Chunk::TYPE_MxSt;
  riff->AppendChild(list);

  for (Children::const_iterator it=GetChildren().begin(); it!=GetChildren().end(); it++) {
    Chunk *mxst = new Chunk(Chunk::TYPE_MxSt);
    list->AppendChild(mxst);

    Object *obj = static_cast<Object*>(*it);
    Chunk *mxob = obj->Export();
    mxst->AppendChild(mxob);

    Chunk *chunklst = new Chunk(Chunk::TYPE_LIST);
    chunklst->data("Format") = Chunk::TYPE_MxDa;
    mxst->AppendChild(chunklst);

    // First, interleave all headers (first chunk)
    for (ssize_t i=-1; i<ssize_t(obj->GetChildCount()); i++) {
      Object *working_obj = static_cast<Object*>((i == -1) ? obj : obj->GetChildAt(i));
      if (!working_obj->data().empty()) {
        const bytearray &data = working_obj->data().front();
        Chunk *mxch = new Chunk(Chunk::TYPE_MxCh);
        mxch->data("Flags") = 0;
        mxch->data("Object") = working_obj->id();
        mxch->data("Time") = 0;
        mxch->data("DataSize") = data.size();
        mxch->data("Data") = data;
        chunklst->AppendChild(mxch);
      }
    }

    // Next, interleave everything by time
    std::vector<ChunkStatus> chunk_status(obj->GetChildCount() + 1);
    bool all_done;
    do {
      all_done = true;
      for (ssize_t i=-1; i<ssize_t(obj->GetChildCount()); i++) {
        Object *working_obj = static_cast<Object*>((i == -1) ? obj : obj->GetChildAt(i));
        ChunkStatus &status = chunk_status[i+1];

        if (status.index < working_obj->data().size()) {
          const bytearray &data = working_obj->data().at(status.index);

          all_done = false;

          Chunk *mxch = new Chunk(Chunk::TYPE_MxCh);
          mxch->data("Flags") = 0;
          mxch->data("Object") = working_obj->id();
          mxch->data("Time") = status.time;
          mxch->data("DataSize") = data.size();
          mxch->data("Data") = data;
          chunklst->AppendChild(mxch);

          status.index++;

          // Increment time
          switch (working_obj->filetype()) {
          case MxOb::WAV:
            status.time += 1000;
            break;
          case MxOb::STL:
            // Unaffected by time
            break;
          }
        }
      }
    } while (!all_done);

    // Finally interleave end chunks
    for (ssize_t i=obj->GetChildCount()-1; i>=-1; i--) {
      Object *working_obj = static_cast<Object*>((i == -1) ? obj : obj->GetChildAt(i));
      ChunkStatus &status = chunk_status[i+1];
      Chunk *mxch = new Chunk(Chunk::TYPE_MxCh);
      mxch->data("Flags") = MxCh::FLAG_END;
      mxch->data("Object") = working_obj->id();
      mxch->data("Time") = status.time;
      mxch->data("DataSize") = 0;
      chunklst->AppendChild(mxch);
    }
  }

  // FIXME: Fill in MxOf table
  // FIXME: Split MxCh chunks over alignment

  return riff;
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

    uint32_t joining_chunk = 0;

    for (Children::const_iterator it=list->GetChildren().begin(); it!=list->GetChildren().end(); it++) {
      Chunk *mxch = static_cast<Chunk*>(*it);
      if (mxch->id() == Chunk::TYPE_pad_) {
        // Ignore this chunk
      } else if (mxch->id() == Chunk::TYPE_MxCh) {
        uint16_t flags = mxch->data("Flags");
        if (!(flags & MxCh::FLAG_END)) {
          uint32_t obj_id = mxch->data("Object");
          const Data &chunk_data = mxch->data("Data");

          // For split chunks, join them together
          if (joining_chunk > 0) {
            data[obj_id].back().append(chunk_data);
            if (data[obj_id].back().size() == joining_chunk) {
              joining_chunk = 0;
            }
          } else {
            if (flags & MxCh::FLAG_SPLIT) {
              joining_chunk = mxch->data("DataSize");
            }

            data[obj_id].push_back(chunk_data);
          }
        }
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
