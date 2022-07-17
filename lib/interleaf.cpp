#include "interleaf.h"

#include <cmath>
#include <iostream>

#include "object.h"
#include "othertypes.h"

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
    list->AppendChild(ExportStream(static_cast<Object*>(*it)));
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

struct ChunkStatus
{
  ChunkStatus()
  {
    object = NULL;
    index = 0;
    time = 0;
    end_chunk = false;
  }

  Object *object;
  size_t index;
  uint32_t time;
  bool end_chunk;
};

Chunk *Interleaf::ExportStream(Object *obj) const
{
  Chunk *mxst = new Chunk(Chunk::TYPE_MxSt);

  Chunk *mxob = obj->Export();
  mxst->AppendChild(mxob);

  Chunk *chunklst = new Chunk(Chunk::TYPE_LIST);
  chunklst->data("Format") = Chunk::TYPE_MxDa;
  mxst->AppendChild(chunklst);

  // Set up chunking status vector
  std::vector<ChunkStatus> chunk_status(obj->GetChildCount() + 1);
  chunk_status[0].object = obj;
  for (size_t i=0; i<obj->GetChildCount(); i++) {
    chunk_status[i+1].object = static_cast<Object*>(obj->GetChildAt(i));
  }

  // First, interleave all headers (first chunk)
  for (std::vector<ChunkStatus>::iterator it=chunk_status.begin(); it!=chunk_status.end(); it++) {
    Object *working_obj = it->object;
    if (!working_obj->data().empty()) {
      chunklst->AppendChild(ExportMxCh(0, working_obj->id(), 0, working_obj->data().front()));
    }
    it->index++;
  }

  // Next, interleave everything by time
  while (true) {
    // Find next chunk
    ChunkStatus *status = NULL;

    for (std::vector<ChunkStatus>::iterator it=chunk_status.begin(); it!=chunk_status.end(); it++) {
      // Check if we've already written all these chunks
      if (it->index >= it->object->data().size()) {
        /*if (!it->end_chunk) {
          chunklst->AppendChild(ExportMxCh(MxCh::FLAG_END, it->object->id(), it->time));
          it->end_chunk = true;
        }*/
        continue;
      }

      // Find earliest chunk to write
      if (!status || it->time < status->time) {
        status = it.base();
      }
    }

    if (!status) {
      // Assume chunks are all done
      break;
    }

    Object *working_obj = status->object;
    const bytearray &data = working_obj->data().at(status->index);

    chunklst->AppendChild(ExportMxCh(0, working_obj->id(), status->time, data));

    status->index++;

    // Increment time
    switch (working_obj->filetype()) {
    case MxOb::WAV:
    {
      const WAVFmt *fmt = working_obj->GetFileHeader().cast<WAVFmt>();
      status->time += (data.size() * 1000) / (fmt->BitsPerSample/8) / fmt->Channels / fmt->SampleRate;
      break;
    }
    case MxOb::SMK:
    {
      int32_t frame_rate = working_obj->GetFileHeader().cast<SMK2>()->FrameRate;
      int32_t fps;
      if (frame_rate > 0) {
        fps = 1000/frame_rate;
      } else if (frame_rate < 0) {
        fps = 100000/-frame_rate;
      } else {
        fps = 10;
      }
      status->time += 1000/fps;
      break;
    }
    case MxOb::FLC:
      status->time += working_obj->GetFileHeader().cast<FLIC>()->speed;
      break;
    case MxOb::STL:
    case MxOb::OBJ:
      // Unaffected by time
      break;
    }
  };

  for (size_t i=1; i<chunk_status.size(); i++) {
    const ChunkStatus &s = chunk_status.at(i);
    chunklst->AppendChild(ExportMxCh(MxCh::FLAG_END, s.object->id(), s.time));
  }
  chunklst->AppendChild(ExportMxCh(MxCh::FLAG_END, chunk_status.front().object->id(), chunk_status.front().time));

  return mxst;
}

Chunk *Interleaf::ExportMxCh(uint16_t flags, uint32_t object_id, uint32_t time, const bytearray &data) const
{
  Chunk *mxch = new Chunk(Chunk::TYPE_MxCh);
  mxch->data("Flags") = flags;
  mxch->data("Object") = object_id;
  mxch->data("Time") = time;
  mxch->data("DataSize") = data.size();
  mxch->data("Data") = data;
  return mxch;
}

}
