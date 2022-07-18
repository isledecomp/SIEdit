#include "object.h"

#include <iostream>

#include "othertypes.h"
#include "util.h"

namespace si {

Object::Object()
{
  type_ = MxOb::Null;
  id_ = 0;
}

#ifdef _WIN32
bool Object::ReplaceWithFile(const wchar_t *f)
{
  std::ifstream is(f);
  if (!is.is_open() || !is.good()) {
    return false;
  }
  return ReplaceWithFile(is);
}

bool Object::ExtractToFile(const wchar_t *f) const
{
  std::ofstream os(f);
  if (!os.is_open() || !os.good()) {
    return false;
  }
  return ExtractToFile(os);
}
#endif

bool Object::ReplaceWithFile(const char *f)
{
  std::ifstream is(f);
  if (!is.is_open() || !is.good()) {
    return false;
  }
  return ReplaceWithFile(is);
}

bool Object::ExtractToFile(const char *f) const
{
  std::ofstream os(f);
  if (!os.is_open() || !os.good()) {
    return false;
  }
  return ExtractToFile(os);
}

bool Object::ReplaceWithFile(std::istream &is)
{
  data_.clear();

  switch (this->filetype()) {
  case MxOb::WAV:
  {
    if (ReadU32(is) != RIFF::RIFF_) {
      return false;
    }

    // Skip total size
    ReadU32(is);

    if (ReadU32(is) != RIFF::WAVE) {
      return false;
    }

    bytearray fmt;
    bytearray data;

    while (is.good()) {
      uint32_t id = ReadU32(is);
      uint32_t sz = ReadU32(is);
      if (id == RIFF::fmt_) {
        fmt.resize(sz);
        is.read(fmt.data(), fmt.size());
      } else if (id == RIFF::data) {
        data.resize(sz);
        is.read(data.data(), data.size());
      } else {
        is.seekg(sz, std::ios::cur);
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
    bytearray hdr(sizeof(SMK2));
    is.read(hdr.data(), hdr.size());

    // Read frame sizes
    SMK2 smk = *hdr.cast<SMK2>();
    bytearray frame_sizes(smk.Frames * sizeof(uint32_t));
    is.read(frame_sizes.data(), frame_sizes.size());
    hdr.append(frame_sizes);

    // Read frame types
    bytearray frame_types(smk.Frames);
    is.read(frame_types.data(), frame_types.size());
    hdr.append(frame_types);

    // Read Huffman trees
    bytearray huffman(smk.TreesSize);
    is.read(huffman.data(), huffman.size());
    hdr.append(huffman);

    // Place header into data vector
    data_.resize(smk.Frames + 1);
    data_[0] = hdr;

    uint32_t *real_sizes = frame_sizes.cast<uint32_t>();
    for (uint32_t i=0; i<smk.Frames; i++) {
      uint32_t sz = real_sizes[i];
      if (sz > 0) {
        bytearray &d = data_[i+1];
        d.resize(sz);
        is.read(d.data(), d.size());
      }
    }
    return true;
  }
  default:
    LogWarning() << "Don't yet know how to chunk type " << RIFF::PrintU32AsString(this->filetype()) << std::endl;
    break;
  }

  return false;
}

bool Object::ExtractToFile(std::ostream &os) const
{
  switch (this->filetype()) {
  case MxOb::WAV:
  {
    // Write RIFF header
    RIFF::Chk riff = RIFF::BeginChunk(os, RIFF::RIFF_);

    WriteU32(os, RIFF::WAVE);

    {
      RIFF::Chk fmt = RIFF::BeginChunk(os, RIFF::fmt_);

      WriteBytes(os, data_.at(0));

      RIFF::EndChunk(os, fmt);
    }

    {
      RIFF::Chk data = RIFF::BeginChunk(os, RIFF::data);
      // Merge all chunks after the first one
      for (size_t i=1; i<data_.size(); i++) {
        WriteBytes(os, data_.at(i));
      }
      RIFF::EndChunk(os, data);
    }

    RIFF::EndChunk(os, riff);
    break;
  }
  case MxOb::STL:
  {
    static const uint32_t BMP_HDR_SZ = 14;

    // Write BMP header
    WriteU16(os, 0x4D42);

    // Write placeholder for size
    std::ios::pos_type sz_loc = os.tellp();
    WriteU32(os, 0);

    // Write "reserved" bytes
    WriteU32(os, 0);

    // Write data offset
    WriteU32(os, data_.at(0).size() + BMP_HDR_SZ);

    for (size_t i=0; i<data_.size(); i++) {
      WriteBytes(os, data_.at(i));
    }

    std::ios::pos_type len = os.tellp();
    os.seekp(sz_loc);
    WriteU32(os, len);
    break;
  }
  case MxOb::FLC:
  {
    // First chunk is a complete FLIC header, so add it as-is
    WriteBytes(os, data_.at(0));

    // Subsequent chunks are FLIC frames with an additional 20 byte header that needs to be stripped
    const int CUSTOM_HEADER_SZ = 20;
    for (size_t i=1; i<data_.size(); i++) {
      os.write(data_.at(i).data() + CUSTOM_HEADER_SZ, data_.at(i).size() - CUSTOM_HEADER_SZ);
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
      WriteBytes(os, data_.at(i));
    }
    break;
  }

  return true;
}

bytearray Object::ExtractToMemory() const
{
  memorybuf buf;
  std::ostream os(&buf);

  ExtractToFile(os);

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
