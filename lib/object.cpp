#include "object.h"

#include <iostream>

#include "othertypes.h"
#include "util.h"

namespace si {

Object::Object()
{
  type_ = MxOb::Null;
  id_ = 0;
  time_offset_ = 0;
}

#ifdef _WIN32
bool Object::ReplaceWithFile(const wchar_t *f)
{
  File is;
  if (!is.Open(f, File::Read)) {
    return false;
  }
  return ReplaceWithFile(&is);
}

bool Object::ExtractToFile(const wchar_t *f) const
{
  File os;
  if (!os.Open(f, File::Write)) {
    return false;
  }
  return ExtractToFile(&os);
}
#endif

bool Object::ReplaceWithFile(const char *f)
{
  File is;
  if (!is.Open(f, File::Read)) {
    return false;
  }
  return ReplaceWithFile(&is);
}

bool Object::ExtractToFile(const char *f) const
{
  File os;
  if (!os.Open(f, File::Write)) {
    return false;
  }
  return ExtractToFile(&os);
}

bool Object::ReplaceWithFile(FileBase *f)
{
  data_.clear();

  switch (this->filetype()) {
  case MxOb::WAV:
  {
    if (f->ReadU32() != RIFF::RIFF_) {
      return false;
    }

    // Skip total size
    f->ReadU32();

    if (f->ReadU32() != RIFF::WAVE) {
      return false;
    }

    bytearray fmt;
    bytearray data;

    while (!f->atEnd()) {
      uint32_t id = f->ReadU32();
      uint32_t sz = f->ReadU32();
      if (id == RIFF::fmt_) {
        fmt = f->ReadBytes(sz);
      } else if (id == RIFF::data) {
        data = f->ReadBytes(sz);
      } else {
        f->seek(sz, File::SeekCurrent);
      }
    }

    if (fmt.empty() || data.empty()) {
      return false;
    }

    data_.push_back(fmt);
    WAVFmt *fmt_info = fmt.cast<WAVFmt>();
    size_t second_in_bytes = fmt_info->Channels * fmt_info->SampleRate * (fmt_info->BitsPerSample/8);
    size_t max;
    for (size_t i=0; i<data.size(); i+=max) {
      max = std::min(data.size() - i, second_in_bytes);
      data_.push_back(bytearray(data.data() + i, max));
    }

    return true;
  }
  case MxOb::SMK:
  {
    // Read header
    bytearray hdr = f->ReadBytes(sizeof(SMK2));

    // Read frame sizes
    SMK2 smk = *hdr.cast<SMK2>();
    bytearray frame_sizes = f->ReadBytes(smk.Frames * sizeof(uint32_t));
    hdr.append(frame_sizes);

    // Read frame types
    hdr.append(f->ReadBytes(smk.Frames));

    // Read Huffman trees
    hdr.append(f->ReadBytes(smk.TreesSize));

    // Place header into data vector
    data_.resize(smk.Frames + 1);
    data_[0] = hdr;

    uint32_t *real_sizes = frame_sizes.cast<uint32_t>();
    for (uint32_t i=0; i<smk.Frames; i++) {
      uint32_t sz = real_sizes[i];
      if (sz > 0) {
        data_[i+1] = f->ReadBytes(sz);
      }
    }
    return true;
  }
  case MxOb::STL:
  {
    BMP bmp;
    f->ReadData(&bmp, sizeof(bmp));

    bytearray info_header = f->ReadBytes(bmp.DataOffset - f->pos());
    data_.push_back(info_header);

    bytearray pixels = f->ReadBytes(bmp.FileSize - f->pos());
    data_.push_back(pixels);

    return true;
  }
  case MxOb::OBJ:
  {
    data_.push_back(f->ReadBytes(f->size()));
    return true;
  }
  default:
    LogWarning() << "Don't yet know how to chunk type " << RIFF::PrintU32AsString(this->filetype()) << std::endl;
    break;
  }

  return false;
}

bool Object::ExtractToFile(FileBase *f) const
{
  if (data_.empty()) {
    return false;
  }

  switch (this->filetype()) {
  case MxOb::WAV:
  {
    // Write RIFF header
    RIFF::Chk riff = RIFF::BeginChunk(f, RIFF::RIFF_);

    f->WriteU32(RIFF::WAVE);

    {
      RIFF::Chk fmt = RIFF::BeginChunk(f, RIFF::fmt_);

      f->WriteBytes(data_.at(0));

      RIFF::EndChunk(f, fmt);
    }

    {
      RIFF::Chk data = RIFF::BeginChunk(f, RIFF::data);
      // Merge all chunks after the first one
      for (size_t i=1; i<data_.size(); i++) {
        f->WriteBytes(data_.at(i));
      }
      RIFF::EndChunk(f, data);
    }

    RIFF::EndChunk(f, riff);
    break;
  }
  case MxOb::STL:
  {
    uint32_t size = sizeof(BMP);
    for (size_t i=0; i<data_.size(); i++) {
      size += data_.at(i).size();
    }

    // Write BMP header
    BMP bmp;
    bmp.Signature = 0x4D42; // 'BM'
    bmp.FileSize = size;
    bmp.Reserved = 0;
    bmp.DataOffset = data_.at(0).size() + sizeof(BMP);

    f->WriteData(&bmp, sizeof(bmp));

    for (size_t i=0; i<data_.size(); i++) {
      f->WriteBytes(data_.at(i));
    }
    break;
  }
  case MxOb::FLC:
  {
    // First chunk is a complete FLIC header, so add it as-is
    f->WriteBytes(data_.at(0));

    // Subsequent chunks are FLIC frames with an additional 20 byte header that needs to be stripped
    const int CUSTOM_HEADER_SZ = 20;
    for (size_t i=1; i<data_.size(); i++) {
      if (data_.at(i).size() == CUSTOM_HEADER_SZ) {
        static const char *empty_hdr = "\x10\x00\x00\x00\xfa\xf1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
        f->WriteData(empty_hdr, 16);
      } else {
        f->WriteData(data_.at(i).data() + CUSTOM_HEADER_SZ, data_.at(i).size() - CUSTOM_HEADER_SZ);
      }
    }
    break;
  }
  default:
    LogWarning() << "Didn't know how to extract type '" << RIFF::PrintU32AsString(filetype()) << "', merging..." << std::endl;
    /* fall-through */
  case MxOb::SMK:
  case MxOb::OBJ:
    // Simply merge
    for (size_t i=0; i<data_.size(); i++) {
      f->WriteBytes(data_.at(i));
    }
    break;
  }

  return true;
}

bytearray Object::ExtractToMemory() const
{
  MemoryBuffer buf;

  ExtractToFile(&buf);

  return buf.data();
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

size_t Object::CalculateMaximumDiskSize() const
{
  size_t s = 0;

  s += 108;
  s += presenter_.size() + 1;
  s += name_.size() + 1;
  s += extra_.size();

  if (type_ != MxOb::Presenter && type_ != MxOb::World && type_ != MxOb::Animation) {
    s += filename_.size() + 1;
    s += 24;

    if (filetype_ == MxOb::WAV) {
      s += 4;
    }
  }

  if (this->HasChildren()) {
    s += 16;

    for (size_t i = 0; i < this->GetChildCount(); i++) {
      s += static_cast<Object*>(this->GetChildAt(i))->CalculateMaximumDiskSize();
    }
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
