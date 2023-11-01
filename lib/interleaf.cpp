#include "interleaf.h"

#include <cmath>
#include <iostream>
#include <sstream>

#include "object.h"
#include "othertypes.h"
#include "sitypes.h"
#include "util.h"

namespace si {

static const uint32_t kMinimumChunkSize = 8;

Interleaf::Interleaf()
{
}

void Interleaf::Clear()
{
  m_Info.clear();
  m_BufferSize = 0;
  m_JoiningProgress = 0;
  m_JoiningSize = 0;
  m_ObjectIDTable.clear();
  m_ObjectList.clear();
  DeleteChildren();
}

Interleaf::Error Interleaf::Read(const char *f)
{
  File is;
  if (!is.Open(f, File::Read)) {
    return ERROR_IO;
  }
  return Read(&is);
}

Interleaf::Error Interleaf::Write(const char *f) const
{
  File os;
  if (!os.Open(f, File::Write)) {
    return ERROR_IO;
  }
  return Write(&os);
}

#ifdef _WIN32
Interleaf::Error Interleaf::Read(const wchar_t *f)
{
  File is;
  if (!is.Open(f, File::Read)) {
    return ERROR_IO;
  }
  return Read(&is);
}

Interleaf::Error Interleaf::Write(const wchar_t *f) const
{
  File os;
  if (!os.Open(f, File::Write)) {
    return ERROR_IO;
  }
  return Write(&os);
}
#endif

Interleaf::Error Interleaf::ReadChunk(Core *parent, FileBase *f, Info *info)
{
  uint32_t offset = f->pos();
  uint32_t id = f->ReadU32();
  uint32_t size = f->ReadU32();
  uint32_t end = uint32_t(f->pos()) + size;

  info->SetType(id);
  info->SetOffset(offset);
  info->SetSize(size);

  std::stringstream desc;

  switch (static_cast<RIFF::Type>(id)) {
  case RIFF::RIFF_:
  {
    // Require RIFF type to be OMNI
    uint32_t riff_type = f->ReadU32();
    if (riff_type != RIFF::OMNI) {
      return ERROR_INVALID_INPUT;
    }

    desc << "Type: " << RIFF::PrintU32AsString(riff_type);
    break;
  }
  case RIFF::MxHd:
  {
    m_Version = f->ReadU32();
    desc << "Version: 0x" << std::hex << m_Version << std::endl;

    m_BufferSize = f->ReadU32();
    desc << "Buffer Size: 0x" << std::hex << m_BufferSize;

    m_BufferCount = f->ReadU32();
    desc << std::endl << "Buffer Count: " << std::dec << m_BufferCount << std::endl;
    break;
  }
  case RIFF::pad_:
    f->seek(size, File::SeekCurrent);
    break;
  case RIFF::MxOf:
  {
    uint32_t offset_count = f->ReadU32();

    desc << "Count: " << offset_count;

    uint32_t real_count = (size - sizeof(uint32_t)) / sizeof(uint32_t);
    m_ObjectList.resize(real_count);
    for (uint32_t i = 0; i < real_count; i++) {
      Object *o = new Object();
      parent->AppendChild(o);

      uint32_t choffset = f->ReadU32();
      m_ObjectList[i] = choffset;
      desc << std::endl << i << ": 0x" << std::hex << choffset;
    }
    break;
  }
  case RIFF::LIST:
  {
    uint32_t list_type = f->ReadU32();
    desc << "Type: " << RIFF::PrintU32AsString(list_type) << std::endl;
    uint32_t list_count = 0;
    if (list_type == RIFF::MxCh) {
      if (m_Version == Version2_1) {
        uint32_t unknown_list_entry = f->ReadU32();
        desc << "Unknown v2.1 list entry: " << unknown_list_entry << std::endl;
      }

      list_count = f->ReadU32();
      if (list_count == LIST::Act_ || list_count == LIST::RAND) {
        desc << "Extension: ";
        if (list_count == LIST::RAND) {
          uint32_t rand_upper = f->ReadU32();
          uint64_t rand_val = uint64_t(rand_upper) << 32 | list_count;
          f->seek(1, File::SeekCurrent);
          desc << ((const char *) &rand_val);
        } else if (list_count == LIST::Act_) {
          desc << ((const char *) &list_count);
        }
        desc << std::endl;

        // Re-read list count
        list_count = f->ReadU32();
        for (uint32_t i=0; i<list_count; i++) {
          // Read every short
          uint16_t val = f->ReadU16();
          desc << "  " << ((const char *) &val) << std::endl;
        }
      }
      desc << "Count: " << list_count << std::endl;
    }
    break;
  }
  case RIFF::MxSt:
  case RIFF::MxDa:
  case RIFF::WAVE:
  case RIFF::fmt_:
  case RIFF::data:
  case RIFF::OMNI:
    // Types with no data
    break;
  case RIFF::MxOb:
  {
    Object *o = NULL;

    for (size_t i=0; i<m_ObjectList.size(); i++) {
      if (m_ObjectList[i] == offset-kMinimumChunkSize) {
        o = static_cast<Object*>(GetChildAt(i));
        break;
      }
    }

    if (!o) {
      o = new Object();
      parent->AppendChild(o);
    }

    ReadObject(f, o, desc);

    info->SetObjectID(o->id());

    m_ObjectIDTable[o->id()] = o;

    parent = o;
    break;
  }
  case RIFF::MxCh:
  {
    uint16_t flags = f->ReadU16();
    desc << "Flags: 0x" << std::hex << flags << std::endl;

    uint32_t object = f->ReadU32();
    desc << "Object: " << std::dec << object << std::endl;

    uint32_t time = f->ReadU32();
    desc << "Time: " << time << std::endl;

    uint32_t data_sz = f->ReadU32();
    desc << "Size: " << data_sz << std::endl;

    bytearray data = f->ReadBytes(size - MxCh::HEADER_SIZE);

    info->SetObjectID(object);
    info->SetData(data);

    if (!(flags & MxCh::FLAG_END)) {
      std::map<uint32_t, Object*>::iterator it = m_ObjectIDTable.find(object);
      if (it == m_ObjectIDTable.end()) {
        LogError() << "Failed to find object " << object << " for chunk at " << std::hex << offset << std::dec << std::endl;
        //return ERROR_INVALID_INPUT;
      } else {
        Object *o = it->second;

        if (flags & MxCh::FLAG_SPLIT && m_JoiningSize > 0) {
          o->data_.back().append(data);

          m_JoiningProgress += data.size();
          if (m_JoiningProgress == m_JoiningSize) {
            m_JoiningProgress = 0;
            m_JoiningSize = 0;
          }
        } else {
          o->data_.push_back(data);

          if (o->data_.size() == 2) {
            o->time_offset_ = time;
          }

          if (flags & MxCh::FLAG_SPLIT) {
            m_JoiningProgress = data.size();
            m_JoiningSize = data_sz;
          }
        }
      }
      break;
    }
  }
  }

  // Assume any remaining data is this chunk's children
  while (!f->atEnd() && (f->pos() + kMinimumChunkSize) < end) {
    // Check alignment, if there's not enough room to for another segment, skip ahead
    if (m_BufferSize > 0) {
      uint32_t offset_in_buffer = f->pos()%m_BufferSize;
      if (offset_in_buffer + kMinimumChunkSize > m_BufferSize) {
        f->seek(m_BufferSize-offset_in_buffer, File::SeekCurrent);
      }
    }

    // Read next child
    Info *subinfo = new Info();
    info->AppendChild(subinfo);
    Error e = ReadChunk(parent, f, subinfo);
    if (e != ERROR_SUCCESS) {
      return e;
    }
  }

  info->SetDescription(desc.str());

  if (f->pos() < end) {
    f->seek(end, File::SeekStart);
  }

  if (size%2 == 1) {
    f->seek(1, File::SeekCurrent);
  }

  return ERROR_SUCCESS;
}

Object *Interleaf::ReadObject(FileBase *f, Object *o, std::stringstream &desc)
{
  o->type_ = static_cast<MxOb::Type>(f->ReadU16());
  desc << "Type: " << o->type_ << std::endl;
  o->presenter_ = f->ReadString();
  desc << "Presenter: " << o->presenter_ << std::endl;
  o->unknown1_ = f->ReadU32();
  desc << "Unknown1: " << o->unknown1_ << std::endl;
  o->name_ = f->ReadString();
  desc << "Name: " << o->name_ << std::endl;
  o->id_ = f->ReadU32();
  desc << "ID: " << o->id_ << std::endl;
  o->flags_ = f->ReadU32();
  desc << "Flags: 0x" << std::hex << o->flags_ << std::dec << std::endl;
  o->unknown4_ = f->ReadU32();
  desc << "Unknown4: " << o->unknown4_ << std::endl;
  o->duration_ = f->ReadU32();
  desc << "Duration: " << o->duration_ << std::endl;
  o->loops_ = f->ReadU32();
  desc << "Loops: " << o->loops_ << std::endl;
  o->location_ = f->ReadVector3();
  desc << "Location: " << o->location_.x << " " << o->location_.y << " " << o->location_.z << std::endl;
  o->direction_ = f->ReadVector3();
  desc << "Direction: " << o->direction_.x << " " << o->direction_.y << " " << o->direction_.z << std::endl;
  o->up_ = f->ReadVector3();
  desc << "Up: " << o->up_.x << " " << o->up_.y << " " << o->up_.z << std::endl;

  uint16_t extra_sz = f->ReadU16();
  desc << "Extra Size: " << extra_sz << std::endl;
  o->extra_ = f->ReadBytes(extra_sz);

  desc << "Extra Data: ";
  if (o->extra_.size() > 0) {
    desc << o->extra_.data() << std::endl;
  }
  desc << std::endl;

  if (o->type_ != MxOb::Presenter && o->type_ != MxOb::World && o->type_ != MxOb::Animation) {
    o->filename_ = f->ReadString();
    desc << "Filename: " << o->filename_ << std::endl;
    o->unknown26_ = f->ReadU32();
    desc << "Unknown26: " << o->unknown26_ << std::endl;
    o->unknown27_ = f->ReadU32();
    desc << "Unknown27: " << o->unknown27_ << std::endl;
    o->unknown28_ = f->ReadU32();
    desc << "Unknown28: " << o->unknown28_ << std::endl;
    o->filetype_ = static_cast<MxOb::FileType>(f->ReadU32());
    desc << "File Type: " << RIFF::PrintU32AsString(o->filetype_) << std::endl;
    o->unknown29_ = f->ReadU32();
    desc << "Unknown29: " << o->unknown29_ << std::endl;
    o->unknown30_ = f->ReadU32();
    desc << "Unknown30: " << o->unknown30_ << std::endl;

    if (o->filetype_ == MxOb::WAV) {
      o->volume_ = f->ReadU32();
      desc << "Unknown31: " << o->volume_ << std::endl;
    }
  }

  return o;
}

Interleaf::Error Interleaf::Read(FileBase *f)
{
  Clear();
  return ReadChunk(this, f, &m_Info);
}

void RecursivelyAddObjectToList(std::vector<Object*> *list, Object *o)
{
  list->push_back(o);
  for (size_t j=0; j<o->GetChildCount(); j++) {
    RecursivelyAddObjectToList(list, static_cast<Object*>(o->GetChildAt(j)));
  }
}

Interleaf::Error Interleaf::Write(FileBase *f) const
{
  if (m_BufferSize == 0) {
    LogError() << "Buffer size must be set to write" << std::endl;
    return ERROR_INVALID_BUFFER_SIZE;
  }

  RIFF::Chk riff = RIFF::BeginChunk(f, RIFF::RIFF_);
  f->WriteU32(RIFF::OMNI);

  size_t offset_table_pos;

  {
    // MxHd
    RIFF::Chk mxhd = RIFF::BeginChunk(f, RIFF::MxHd);

    f->WriteU32(m_Version);
    f->WriteU32(m_BufferSize);
    f->WriteU32(m_BufferCount);

    RIFF::EndChunk(f, mxhd);
  }

  {
    // MxOf
    RIFF::Chk mxof = RIFF::BeginChunk(f, RIFF::MxOf);

    f->WriteU32(GetChildCount());

    offset_table_pos = f->pos();

    for (size_t i = 0; i < GetChildCount(); i++) {
      f->WriteU32(0);
    }

    RIFF::EndChunk(f, mxof);
  }

  {
    // LIST
    RIFF::Chk list_mxst = RIFF::BeginChunk(f, RIFF::LIST);

    f->WriteU32(RIFF::MxSt);

    for (size_t i = 0; i < GetChildCount(); i++) {
      Object *child = static_cast<Object*>(GetChildAt(i));
      if (child->type() == MxOb::Null) {
        continue;
      }

      size_t maxSz = child->CalculateMaximumDiskSize() + kMinimumChunkSize;
      WritePaddingIfNecessary(f, maxSz);

      uint32_t mxst_offset = f->pos();

      f->seek(size_t(offset_table_pos) + i * sizeof(uint32_t));
      f->WriteU32(mxst_offset);
      f->seek(mxst_offset);

      // MxSt
      RIFF::Chk mxst = RIFF::BeginChunk(f, RIFF::MxSt);

      {
        // MxOb
        WriteObject(f, child);
      }

      {
        // LIST
        RIFF::Chk list_mxda = RIFF::BeginChunk(f, RIFF::LIST);

        f->WriteU32(RIFF::MxDa);

        // First, interleave headers
        std::vector<Object*> objects;
        objects.reserve(child->GetChildCount() + 1);
        RecursivelyAddObjectToList(&objects, child);

        InterleaveObjects(f, objects);

        RIFF::EndChunk(f, list_mxda);
      }

      RIFF::EndChunk(f, mxst);
    }

    // Fill remainder with padding
    if (f->pos()%m_BufferSize != 0) {
      uint32_t current_buf = f->pos() / m_BufferSize;
      uint32_t target_sz = (current_buf + 1) * m_BufferSize;

      WritePadding(f, target_sz - f->pos());
    }

    RIFF::EndChunk(f, list_mxst);
  }

  RIFF::EndChunk(f, riff);

  return ERROR_SUCCESS;
}

void Interleaf::WriteObject(FileBase *f, const Object *o) const
{
  WritePaddingIfNecessary(f, o->CalculateMaximumDiskSize());

  RIFF::Chk mxob = RIFF::BeginChunk(f, RIFF::MxOb);

  f->WriteU16(o->type_);
  f->WriteString(o->presenter_);
  f->WriteU32(o->unknown1_);
  f->WriteString(o->name_);
  f->WriteU32(o->id_);
  f->WriteU32(o->flags_);
  f->WriteU32(o->unknown4_);
  f->WriteU32(o->duration_);
  f->WriteU32(o->loops_);
  f->WriteVector3(o->location_);
  f->WriteVector3(o->direction_);
  f->WriteVector3(o->up_);

  f->WriteU16(o->extra_.size());
  f->WriteBytes(o->extra_);

  if (o->type_ != MxOb::Presenter && o->type_ != MxOb::World && o->type_ != MxOb::Animation) {
    f->WriteString(o->filename_);
    f->WriteU32(o->unknown26_);
    f->WriteU32(o->unknown27_);
    f->WriteU32(o->unknown28_);
    f->WriteU32(o->filetype_);
    f->WriteU32(o->unknown29_);
    f->WriteU32(o->unknown30_);

    if (o->filetype_ == MxOb::WAV) {
      f->WriteU32(o->volume_);
    }
  }

  if (o->HasChildren()) {
    // Child list
    RIFF::Chk list_mxch = RIFF::BeginChunk(f, RIFF::LIST);

    f->WriteU32(RIFF::MxCh);
    f->WriteU32(o->GetChildCount());

    for (size_t i = 0; i < o->GetChildCount(); i++) {
      WriteObject(f, static_cast<Object*>(o->GetChildAt(i)));
    }

    RIFF::EndChunk(f, list_mxch);
  }

  RIFF::EndChunk(f, mxob);
}

struct ChunkStatus
{
  Object *object;
  size_t index;
  uint32_t time;
};

bool HasChildrenThatNeedPriority(Object *parent, uint32_t parent_time, const std::vector<ChunkStatus> &other_jobs)
{
  for (size_t i=0; i<other_jobs.size(); i++) {
    Object *other_obj = other_jobs.at(i).object;

    if (parent->ContainsChild(other_obj) && other_jobs.at(i).time <= parent_time) {
      return true;
    }
  }
  return false;
}

void Interleaf::InterleaveObjects(FileBase *f, const std::vector<Object *> &objects) const
{
  std::vector<ChunkStatus> status(objects.size());

  // Set up status vector
  for (size_t i=0; i<objects.size(); i++) {
    status[i].object = objects.at(i);
    status[i].index = 0;
    status[i].time = status[i].object->time_offset_;
  }

  // First, interleave headers
  for (std::vector<ChunkStatus>::iterator it = status.begin(); it != status.end(); ) {
    ChunkStatus &s = *it;
    Object *o = s.object;

    bool proceed = true;

    if (!o->data().empty()) {
      WriteSubChunk(f, 0, o->id(), 0xFFFFFFFF, o->data().front());
      s.index++;

      // If we've already reached the end, write the end chunk now
      if (o->data().size() == s.index) {
        WriteSubChunk(f, MxCh::FLAG_END, o->id(), 0xFFFFFFFF);
        it = status.erase(it);
        proceed = false;
      }
    }

    if (proceed) {
      it++;
    }
  }

  // Next, interleave the rest based on time
  while (true) {
    // Update parent time too
    bool done = false;
    while (!done) {
      done = true;
      for (size_t j=0; j<status.size(); j++) {
        ChunkStatus &s = status.at(j);
        Object *obj = s.object;

        for (size_t i=0; i<status.size(); i++) {
          ChunkStatus &p = status.at(i);
          if (p.object != obj) {
            if (p.object->ContainsChild(obj)) {
              if (p.time < s.time) {
                p.time = s.time;
                done = false;
              }
            }
          }
        }
      }
    }

    // Find next chunk
    std::vector<ChunkStatus>::iterator s = status.begin();
    if (s == status.end()) {
      break;
    }

    while (HasChildrenThatNeedPriority(s->object, s->time, status)) {
      s++;
    }

    if (s == status.end()) {
      break;
    }

    std::vector<ChunkStatus>::iterator it = s;
    it++;
    for (; it!=status.end(); it++) {
      // Find earliest chunk to write
      if (it->time < s->time && !HasChildrenThatNeedPriority(it->object, it->time, status)) {
        s = it;
      }
    }

    if (s->index == s->object->data_.size()) {
      WriteSubChunk(f, MxCh::FLAG_END, s->object->id(), s->time);
      status.erase(s);
      continue;
    }

    Object *obj = s->object;
    const bytearray &data = obj->data().at(s->index);

    WriteSubChunk(f, 0, obj->id(), s->time, data);

    s->index++;

    // Increment time
    switch (obj->filetype()) {
    case MxOb::WAV:
    {
      const WAVFmt *fmt = obj->GetFileHeader().cast<WAVFmt>();
      s->time += round(double(data.size() * 1000) / (fmt->BitsPerSample/8) / fmt->Channels / fmt->SampleRate);
      break;
    }
    case MxOb::SMK:
    {
      int32_t frame_rate = obj->GetFileHeader().cast<SMK2>()->FrameRate;
      int32_t fps;
      if (frame_rate > 0) {
        fps = 1000/frame_rate;
      } else if (frame_rate < 0) {
        fps = 100000/-frame_rate;
      } else {
        fps = 10;
      }
      s->time += 1000/fps;
      break;
    }
    case MxOb::FLC:
      s->time += obj->GetFileHeader().cast<FLIC>()->speed;
      break;
    case MxOb::STL:
    case MxOb::OBJ:
      // Unaffected by time
      break;
    }
  }
}

void Interleaf::WriteSubChunk(FileBase *f, uint16_t flags, uint32_t object, uint32_t time, const bytearray &data) const
{
  static const uint32_t total_hdr = MxCh::HEADER_SIZE + kMinimumChunkSize;

  uint32_t data_offset = 0;

  while (data_offset < data.size() || data.size() == 0) {
    uint32_t data_sz = data.size() - data_offset;

    // Calculate whether this chunk will overrun the buffer
    uint32_t start_buffer = f->pos() / m_BufferSize;
    uint32_t stop_buffer = (uint32_t(f->pos()) - 1 + data_sz + total_hdr) / m_BufferSize;

    size_t max_chunk = data_sz;

    if (start_buffer != stop_buffer) {
      size_t remaining = ((start_buffer + 1) * m_BufferSize) - f->pos();

      if (remaining < total_hdr) {
        if (remaining < kMinimumChunkSize) {
          // There isn't enough space for another chunk, just jump ahead
          f->seek(remaining, File::SeekCurrent);
        } else {
          // This chunk won't fit in our buffer alignment. We must make a decision to either insert
          // padding or split the clip.
          WritePadding(f, remaining);
        }
        continue;
      }

      max_chunk = remaining - total_hdr;

      if (!(flags & MxCh::FLAG_SPLIT)) {

        // FIXME: Not sure exactly what this value is yet, likely to be smaller than this
        static const uint32_t MAX_PADDING = 9882;

        if (remaining < MAX_PADDING) {
          // This chunk won't fit in our buffer alignment. We must make a decision to either insert
          // padding or split the clip.
          WritePadding(f, remaining);

          // Do loop over again
          continue;
        } else {
          flags |= MxCh::FLAG_SPLIT;
        }
      }
    }

    bytearray chunk = data.mid(data_offset, max_chunk);
    WriteSubChunkInternal(f, flags, object, time, data_sz, chunk);
    data_offset += chunk.size();

    if (data.size() == 0) {
      break;
    }
  }
}

void Interleaf::WriteSubChunkInternal(FileBase *f, uint16_t flags, uint32_t object, uint32_t time, uint32_t data_sz, const bytearray &data) const
{
  RIFF::Chk mxch = RIFF::BeginChunk(f, RIFF::MxCh);

  f->WriteU16(flags);
  f->WriteU32(object);
  f->WriteU32(time);
  f->WriteU32(data_sz);
  f->WriteBytes(data);

  RIFF::EndChunk(f, mxch);
}

void Interleaf::WritePadding(FileBase *f, uint32_t size) const
{
  if (size < kMinimumChunkSize) {
    return;
  }

  size -= kMinimumChunkSize;

  f->WriteU32(RIFF::pad_);
  f->WriteU32(size);

  bytearray b(size);
  b.fill(0xCD);
  f->WriteBytes(b);
}

void Interleaf::WritePaddingIfNecessary(FileBase *f, size_t projectedWrite) const
{
  size_t projected_end = f->pos() + projectedWrite;
  size_t this_buf = f->pos()/m_BufferSize;
  size_t end_buf = projected_end/m_BufferSize;
  if (this_buf != end_buf) {
    WritePadding(f, (end_buf * m_BufferSize) - f->pos());
  }
}

}
