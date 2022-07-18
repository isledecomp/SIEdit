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
  std::ifstream is(f);
  if (!is.is_open() || !is.good()) {
    return ERROR_IO;
  }
  return Read(is);
}

Interleaf::Error Interleaf::Write(const char *f) const
{
  std::ofstream os(f);
  if (!os.is_open() || !os.good()) {
    return ERROR_IO;
  }
  return Write(os);
}

#ifdef _WIN32
Interleaf::Error Interleaf::Read(const wchar_t *f)
{
  std::istream is(f);
  if (!is.is_open() || !is.good()) {
    return false;
  }
  return Read(is);
}

Interleaf::Error Interleaf::Write(const wchar_t *f) const
{
  std::ostream os(f);
  if (!os.is_open() || !os.good()) {
    return false;
  }
  return Write(os);
}
#endif

Interleaf::Error Interleaf::ReadChunk(Core *parent, std::istream &is, Info *info)
{
  uint32_t offset = is.tellg();
  uint32_t id = ReadU32(is);
  uint32_t size = ReadU32(is);
  uint32_t end = uint32_t(is.tellg()) + size;

  info->SetType(id);
  info->SetOffset(offset);
  info->SetSize(size);

  std::stringstream desc;

  switch (static_cast<RIFF::Type>(id)) {
  case RIFF::RIFF_:
  {
    // Require RIFF type to be OMNI
    uint32_t riff_type = ReadU32(is);
    if (riff_type != RIFF::OMNI) {
      return ERROR_INVALID_INPUT;
    }

    desc << "Type: " << RIFF::PrintU32AsString(riff_type);
    break;
  }
  case RIFF::MxHd:
  {
    m_Version = ReadU32(is);
    desc << "Version: " << m_Version << std::endl;

    m_BufferSize = ReadU32(is);
    desc << "Buffer Size: 0x" << std::hex << m_BufferSize;

    if (m_Version == 0x00020002) {
      m_BufferCount = ReadU32(is);
      desc << std::endl << "Buffer Count: " << std::dec << m_BufferCount << std::endl;
    }
    break;
  }
  case RIFF::pad_:
    is.seekg(size, std::ios::cur);
    break;
  case RIFF::MxOf:
  {
    uint32_t offset_count = ReadU32(is);

    desc << "Count: " << offset_count;

    uint32_t real_count = (size - sizeof(uint32_t)) / sizeof(uint32_t);
    m_ObjectList.resize(real_count);
    for (uint32_t i = 0; i < real_count; i++) {
      Object *o = new Object();
      parent->AppendChild(o);

      uint32_t choffset = ReadU32(is);
      m_ObjectList[i] = choffset;
      desc << std::endl << i << ": 0x" << std::hex << choffset;
    }
    break;
  }
  case RIFF::LIST:
  {
    uint32_t list_type = ReadU32(is);
    desc << "Type: " << RIFF::PrintU32AsString(list_type) << std::endl;
    uint32_t list_count = 0;
    if (list_type == RIFF::MxCh) {
      list_count = ReadU32(is);
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

    ReadObject(is, o, desc);

    info->SetObjectID(o->id());

    m_ObjectIDTable[o->id()] = o;

    parent = o;
    break;
  }
  case RIFF::MxCh:
  {
    uint16_t flags = ReadU16(is);
    desc << "Flags: 0x" << std::hex << flags << std::endl;

    uint32_t object = ReadU32(is);
    desc << "Object: " << std::dec << object << std::endl;

    uint32_t time = ReadU32(is);
    desc << "Time: " << time << std::endl;

    uint32_t data_sz = ReadU32(is);
    desc << "Size: " << data_sz << std::endl;

    bytearray data = ReadBytes(is, size - MxCh::HEADER_SIZE);

    info->SetObjectID(object);
    info->SetData(data);

    if (!(flags & MxCh::FLAG_END)) {
      Object *o = m_ObjectIDTable.at(object);
      if (!o) {
        return ERROR_INVALID_INPUT;
      }

      if (flags & MxCh::FLAG_SPLIT && m_JoiningSize > 0) {
        o->data_.back().append(data);

        m_JoiningProgress += data.size();
        if (m_JoiningProgress == m_JoiningSize) {
          m_JoiningProgress = 0;
          m_JoiningSize = 0;
        }
      } else {
        o->data_.push_back(data);

        if (flags & MxCh::FLAG_SPLIT) {
          m_JoiningProgress = data.size();
          m_JoiningSize = data_sz;
        }
      }
      break;
    }
  }
  }

  // Assume any remaining data is this chunk's children
  while (is.good() && (size_t(is.tellg()) + kMinimumChunkSize) < end) {
    // Check alignment, if there's not enough room to for another segment, skip ahead
    if (m_BufferSize > 0) {
      uint32_t offset_in_buffer = is.tellg()%m_BufferSize;
      if (offset_in_buffer + kMinimumChunkSize > m_BufferSize) {
        is.seekg(m_BufferSize-offset_in_buffer, std::ios::cur);
      }
    }

    // Read next child
    Info *subinfo = new Info();
    info->AppendChild(subinfo);
    Error e = ReadChunk(parent, is, subinfo);
    if (e != ERROR_SUCCESS) {
      return e;
    }
  }

  info->SetDescription(desc.str());

  if (is.tellg() < end) {
    is.seekg(end, std::ios::beg);
  }

  if (size%2 == 1) {
    is.seekg(1, std::ios::cur);
  }

  return ERROR_SUCCESS;
}

Object *Interleaf::ReadObject(std::istream &is, Object *o, std::stringstream &desc)
{
  o->type_ = static_cast<MxOb::Type>(ReadU16(is));
  desc << "Type: " << o->type_ << std::endl;
  o->presenter_ = ReadString(is);
  desc << "Presenter: " << o->presenter_ << std::endl;
  o->unknown1_ = ReadU32(is);
  desc << "Unknown1: " << o->unknown1_ << std::endl;
  o->name_ = ReadString(is);
  desc << "Name: " << o->name_ << std::endl;
  o->id_ = ReadU32(is);
  desc << "ID: " << o->id_ << std::endl;
  o->flags_ = ReadU32(is);
  desc << "Flags: " << o->flags_ << std::endl;
  o->unknown4_ = ReadU32(is);
  desc << "Unknown4: " << o->unknown4_ << std::endl;
  o->duration_ = ReadU32(is);
  desc << "Duration: " << o->duration_ << std::endl;
  o->loops_ = ReadU32(is);
  desc << "Loops: " << o->loops_ << std::endl;
  o->position_ = ReadVector3(is);
  desc << "Position: " << o->position_.x << " " << o->position_.y << " " << o->position_.z << std::endl;
  o->direction_ = ReadVector3(is);
  desc << "Direction: " << o->direction_.x << " " << o->direction_.y << " " << o->direction_.z << std::endl;
  o->up_ = ReadVector3(is);
  desc << "Up: " << o->up_.x << " " << o->up_.y << " " << o->up_.z << std::endl;

  uint16_t extra_sz = ReadU16(is);
  desc << "Extra Size: " << extra_sz << std::endl;
  o->extra_ = ReadBytes(is, extra_sz);

  if (o->type_ != MxOb::Presenter && o->type_ != MxOb::World) {
    o->filename_ = ReadString(is);
    desc << "Filename: " << o->filename_ << std::endl;
    o->unknown26_ = ReadU32(is);
    desc << "Unknown26: " << o->unknown26_ << std::endl;
    o->unknown27_ = ReadU32(is);
    desc << "Unknown27: " << o->unknown27_ << std::endl;
    o->unknown28_ = ReadU32(is);
    desc << "Unknown28: " << o->unknown28_ << std::endl;
    o->filetype_ = static_cast<MxOb::FileType>(ReadU32(is));
    desc << "File Type: " << RIFF::PrintU32AsString(o->filetype_) << std::endl;
    o->unknown29_ = ReadU32(is);
    desc << "Unknown29: " << o->unknown29_ << std::endl;
    o->unknown30_ = ReadU32(is);
    desc << "Unknown30: " << o->unknown30_ << std::endl;

    if (o->filetype_ == MxOb::WAV) {
      o->unknown31_ = ReadU32(is);
      desc << "Unknown31: " << o->unknown31_ << std::endl;
    }
  }

  return o;
}

Interleaf::Error Interleaf::Read(std::istream &is)
{
  Clear();
  return ReadChunk(this, is, &m_Info);
}

Interleaf::Error Interleaf::Write(std::ostream &os) const
{
  if (m_BufferSize == 0) {
    LogError() << "Buffer size must be set to write" << std::endl;
    return ERROR_INVALID_BUFFER_SIZE;
  }

  RIFF::Chk riff = RIFF::BeginChunk(os, RIFF::RIFF_);
  WriteU32(os, RIFF::OMNI);

  std::ios::pos_type offset_table_pos;

  {
    // MxHd
    RIFF::Chk mxhd = RIFF::BeginChunk(os, RIFF::MxHd);

    WriteU32(os, m_Version);
    WriteU32(os, m_BufferSize);

    if (m_Version == 0x00020002) {
      WriteU32(os, m_BufferCount);
    }

    RIFF::EndChunk(os, mxhd);
  }

  {
    // MxOf
    RIFF::Chk mxof = RIFF::BeginChunk(os, RIFF::MxOf);

    WriteU32(os, GetChildCount());

    offset_table_pos = os.tellp();

    for (size_t i = 0; i < GetChildCount(); i++) {
      WriteU32(os, 0);
    }

    RIFF::EndChunk(os, mxof);
  }

  {
    // LIST
    RIFF::Chk list_mxst = RIFF::BeginChunk(os, RIFF::LIST);

    WriteU32(os, RIFF::MxSt);

    for (size_t i = 0; i < GetChildCount(); i++) {
      Object *child = static_cast<Object*>(GetChildAt(i));

      uint32_t mxst_offset = os.tellp();

      os.seekp(size_t(offset_table_pos) + i * sizeof(uint32_t));
      WriteU32(os, mxst_offset);
      os.seekp(mxst_offset);

      // MxSt
      RIFF::Chk mxst = RIFF::BeginChunk(os, RIFF::MxSt);

      {
        // MxOb
        WriteObject(os, child);
      }

      {
        // LIST
        RIFF::Chk list_mxda = RIFF::BeginChunk(os, RIFF::LIST);

        WriteU32(os, RIFF::MxDa);

        // First, interleave headers
        std::vector<Object*> objects;
        objects.reserve(child->GetChildCount() + 1);
        objects.push_back(child);
        for (size_t j=0; j<child->GetChildCount(); j++) {
          objects.push_back(static_cast<Object*>(child->GetChildAt(j)));
        }

        InterleaveObjects(os, objects);

        RIFF::EndChunk(os, list_mxda);
      }

      RIFF::EndChunk(os, mxst);
    }

    // Fill remainder with padding
    if (os.tellp()%m_BufferSize != 0) {
      uint32_t current_buf = os.tellp() / m_BufferSize;
      uint32_t target_sz = (current_buf + 1) * m_BufferSize;

      WritePadding(os, target_sz - os.tellp());
    }

    RIFF::EndChunk(os, list_mxst);
  }

  RIFF::EndChunk(os, riff);

  return ERROR_SUCCESS;
}

void Interleaf::WriteObject(std::ostream &os, const Object *o) const
{
  RIFF::Chk mxob = RIFF::BeginChunk(os, RIFF::MxOb);

  WriteU16(os, o->type_);
  WriteString(os, o->presenter_);
  WriteU32(os, o->unknown1_);
  WriteString(os, o->name_);
  WriteU32(os, o->id_);
  WriteU32(os, o->flags_);
  WriteU32(os, o->unknown4_);
  WriteU32(os, o->duration_);
  WriteU32(os, o->loops_);
  WriteVector3(os, o->position_);
  WriteVector3(os, o->direction_);
  WriteVector3(os, o->up_);

  WriteU16(os, o->extra_.size());
  WriteBytes(os, o->extra_);

  if (o->type_ != MxOb::Presenter && o->type_ != MxOb::World) {
    WriteString(os, o->filename_);
    WriteU32(os, o->unknown26_);
    WriteU32(os, o->unknown27_);
    WriteU32(os, o->unknown28_);
    WriteU32(os, o->filetype_);
    WriteU32(os, o->unknown29_);
    WriteU32(os, o->unknown30_);

    if (o->filetype_ == MxOb::WAV) {
      WriteU32(os, o->unknown31_);
    }
  }

  if (o->HasChildren()) {
    // Child list
    RIFF::Chk list_mxch = RIFF::BeginChunk(os, RIFF::LIST);

    WriteU32(os, RIFF::MxCh);
    WriteU32(os, o->GetChildCount());

    for (size_t i = 0; i < o->GetChildCount(); i++) {
      WriteObject(os, static_cast<Object*>(o->GetChildAt(i)));
    }

    RIFF::EndChunk(os, list_mxch);
  }

  RIFF::EndChunk(os, mxob);
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
    if (parent->ContainsChild(other_jobs.at(i).object)
        && other_jobs.at(i).time <= parent_time) {
      return true;
    }
  }
  return false;
}

void Interleaf::InterleaveObjects(std::ostream &os, const std::vector<Object *> &objects) const
{
  std::vector<ChunkStatus> status(objects.size());

  // Set up status vector
  for (size_t i=0; i<objects.size(); i++) {
    status[i].object = objects.at(i);
    status[i].index = 0;
    status[i].time = 0;
  }

  // First, interleave headers
  for (size_t i=0; i<status.size(); i++) {
    ChunkStatus &s = status[i];
    Object *o = s.object;
    if (!o->data().empty()) {
      WriteSubChunk(os, 0, o->id(), 0xFFFFFFFF, o->data().front());
      s.index++;
    }
  }

  // Next, interleave the rest based on time
  while (true) {
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
      WriteSubChunk(os, MxCh::FLAG_END, s->object->id(), s->time);
      status.erase(s);
      continue;
    }

    Object *obj = s->object;
    const bytearray &data = obj->data().at(s->index);

    WriteSubChunk(os, 0, obj->id(), s->time, data);

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

    // Update parent time too
    for (size_t i=0; i<status.size(); i++) {
      ChunkStatus &p = status.at(i);
      if (p.object != obj) {
        if (p.object->ContainsChild(obj)) {
          p.time = std::max(p.time, s->time);
        }
      }
    }
  }
}

void Interleaf::WriteSubChunk(std::ostream &os, uint16_t flags, uint32_t object, uint32_t time, const bytearray &data) const
{
  static const uint32_t total_hdr = MxCh::HEADER_SIZE + kMinimumChunkSize;

  uint32_t data_offset = 0;

  while (data_offset < data.size() || data.size() == 0) {
    uint32_t data_sz = data.size() - data_offset;

    // Calculate whether this chunk will overrun the buffer
    uint32_t start_buffer = os.tellp() / m_BufferSize;
    uint32_t stop_buffer = (uint32_t(os.tellp()) - 1 + data_sz + total_hdr) / m_BufferSize;

    size_t max_chunk = data_sz;

    if (start_buffer != stop_buffer) {
      size_t remaining = ((start_buffer + 1) * m_BufferSize) - os.tellp();
      max_chunk = remaining - total_hdr;

      if (!(flags & MxCh::FLAG_SPLIT)) {
        if (remaining < 9882) {
          // This chunk won't fit in our buffer alignment. We must make a decision to either insert
          // padding or split the clip.
          WritePadding(os, remaining);

          // Do loop over again
          continue;
        } else {
          flags |= MxCh::FLAG_SPLIT;
        }
      }
    }

    bytearray chunk = data.mid(data_offset, max_chunk);
    WriteSubChunkInternal(os, flags, object, time, data_sz, chunk);
    data_offset += chunk.size();

    if (data.size() == 0) {
      break;
    }
  }
}

void Interleaf::WriteSubChunkInternal(std::ostream &os, uint16_t flags, uint32_t object, uint32_t time, uint32_t data_sz, const bytearray &data) const
{
  RIFF::Chk mxch = RIFF::BeginChunk(os, RIFF::MxCh);

  WriteU16(os, flags);
  WriteU32(os, object);
  WriteU32(os, time);
  WriteU32(os, data_sz);
  WriteBytes(os, data);

  RIFF::EndChunk(os, mxch);
}

void Interleaf::WritePadding(std::ostream &os, uint32_t size) const
{
  if (size < kMinimumChunkSize) {
    return;
  }

  size -= kMinimumChunkSize;

  WriteU32(os, RIFF::pad_);
  WriteU32(os, size);

  bytearray b(size);
  b.fill(0xCD);
  WriteBytes(os, b);
}

}
