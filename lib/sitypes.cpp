#include "sitypes.h"

#include "chunk.h"

namespace si {

Data ReadU32(std::ifstream &is)
{
  uint32_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

void WriteU32(std::ofstream &os, uint32_t u)
{
  os.write((const char *) &u, sizeof(u));
}

Data ReadU16(std::ifstream &is)
{
  uint16_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

void WriteU16(std::ofstream &os, uint16_t u)
{
  os.write((const char *) &u, sizeof(u));
}

Data ReadU8(std::ifstream &is)
{
  uint8_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

void WriteU8(std::ofstream &os, uint8_t u)
{
  os.write((const char *) &u, sizeof(u));
}

Data ReadVector3(std::ifstream &is)
{
  Vector3 u;
  is.read((char *) &u, sizeof(u));
  return u;
}

void WriteVector3(std::ofstream &os, Vector3 v)
{
  os.write((const char *) &v, sizeof(v));
}

Data ReadString(std::ifstream &is)
{
  bytearray d;

  while (true) {
    char c;
    is.read(&c, 1);
    if (c == 0) {
      break;
    }
    d.push_back(c);
  }

  // Append null terminator
  d.push_back(0);

  return d;
}

void WriteString(std::ofstream &os, const std::string &d)
{
  if (!d.empty()) {
    // Write every byte that isn't null
    const char *s = &d[0];
    while ((*s) != 0) {
      os.write(s, 1);
    }
  }

  // Ensure null terminator
  const char nullterm = 0;
  os.write(&nullterm, 1);
}

Data ReadBytes(std::ifstream &is, size_t size)
{
  bytearray d;

  d.resize(size);
  is.read(d.data(), size);

  return d;
}

void WriteBytes(std::ofstream &os, const bytearray &ba)
{
  os.write(ba.data(), ba.size());
}

void RIFF::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Format"] = ReadU32(is);
}

void RIFF::Write(std::ofstream &os, const DataMap &data, uint32_t version)
{
  WriteU32(os, data.at("Format"));
}

void LIST::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Format"] = ReadU32(is);

  if (data["Format"] == Chunk::TYPE_MxCh) {
    data["Count"] = ReadU32(is);
  }
}

void LIST::Write(std::ofstream &os, const DataMap &data, uint32_t version)
{
  WriteU32(os, data.at("Format"));

  if (data.at("Format") == Chunk::TYPE_MxCh) {
    WriteU32(os, data.at("Count"));
  }
}

void MxSt::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  // MxSt is a container type only and has no members, so nothing needs to be done here
}

void MxSt::Write(std::ofstream &os, const DataMap &data, uint32_t version)
{
  // MxSt is a container type only and has no members, so nothing needs to be done here
}

void MxHd::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  Data v = ReadU32(is);
  data["Version"] = v;
  data["BufferSize"] = ReadU32(is);
  data["BufferCount"] = ReadU32(is);
}

void MxHd::Write(std::ofstream &os, const DataMap &data, uint32_t version)
{
  WriteU32(os, data.at("Version"));
  WriteU32(os, data.at("BufferSize"));
  WriteU32(os, data.at("BufferCount"));
}

void MxCh::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Flags"] = ReadU16(is);
  data["Object"] = ReadU32(is);
  data["Time"] = ReadU32(is);
  data["DataSize"] = ReadU32(is);
  data["Data"] = ReadBytes(is, size - 0xE);
}

void MxCh::Write(std::ofstream &os, const DataMap &data, uint32_t version)
{
  WriteU16(os, data.at("Flags"));
  WriteU32(os, data.at("Object"));
  WriteU32(os, data.at("Time"));
  WriteU32(os, data.at("DataSize"));
  WriteBytes(os, data.at("Data"));
}

void MxOf::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Count"] = ReadU32(is);
  data["Offsets"] = ReadBytes(is, size - sizeof(uint32_t));
}

void MxOf::Write(std::ofstream &os, const DataMap &data, uint32_t version)
{
  WriteU32(os, data.at("Count"));
  WriteBytes(os, data.at("Offsets"));
}

void pad_::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  is.seekg(size, std::ios::cur);
}

void pad_::WritePadding(std::ofstream &os, size_t size)
{
  bytearray b;
  b.resize(size);
  b.fill(0xCD);
  WriteBytes(os, b);
}

const char *MxOb::GetTypeName(Type type)
{
  switch (type) {
  case Video:
    return "SMK";
  case Sound:
    return "WAV";
  case Presenter:
    return "MxPresenter";
  case BMP:
    return "BMP";
  case OBJ:
    return "3D Object";
  case World:
    return "World";
  case Event:
    return "Event";
  case Animation:
    return "Animation";
  case TYPE_COUNT:
    break;
  }

  return "Unknown";
}

std::vector<const char*> MxOb::GetFlagsName(Flags flags)
{
  std::vector<const char*> names;

  if (flags == FLAGS_COUNT) {
    return names;
  }

  if (flags & Transparent) {
    names.push_back("Transparent");
  }
  if (flags & NoLoop) {
    names.push_back("NoLoop");
  }
  if (flags & LoopCache) {
    names.push_back("LoopCache");
  }
  if (flags & LoopStream) {
    names.push_back("LoopStream");
  }
  if (flags & Unknown) {
    names.push_back("Unknown");
  }

  return names;
}

void MxOb::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  Data obj_type = ReadU16(is);
  data["Type"] = obj_type;
  data["Presenter"] = ReadString(is);
  data["Unknown1"] = ReadU32(is);
  data["Name"] = ReadString(is);
  data["ID"] = ReadU32(is);
  data["Flags"] = ReadU32(is);
  data["Unknown4"] = ReadU32(is);
  data["Duration"] = ReadU32(is);
  data["Loops"] = ReadU32(is);
  data["Position"] = ReadVector3(is);
  data["Direction"] = ReadVector3(is);
  data["Up"] = ReadVector3(is);

  Data extra_sz = ReadU16(is);
  data["ExtraLength"] = extra_sz;
  data["ExtraData"] = ReadBytes(is, extra_sz);

  if (obj_type != Presenter && obj_type != World) {
    data["FileName"] = ReadString(is);

    data["Unknown26"] = ReadU32(is);
    data["Unknown27"] = ReadU32(is);
    data["Unknown28"] = ReadU32(is);

    data["FileType"] = ReadU32(is);

    data["Unknown29"] = ReadU32(is);
    data["Unknown30"] = ReadU32(is);

    if (obj_type == MxOb::Sound) {
      data["Unknown31"] = ReadU32(is);
    }
  }
}

void MxOb::Write(std::ofstream &os, const DataMap &data, uint32_t version)
{
  const Data &obj_type = data.at("Type");
  WriteU16(os, obj_type);
  WriteString(os, data.at("Presenter"));
  WriteU32(os, data.at("Unknown1"));
  WriteString(os, data.at("Name"));
  WriteU32(os, data.at("ID"));
  WriteU32(os, data.at("Flags"));
  WriteU32(os, data.at("Unknown4"));
  WriteU32(os, data.at("Duration"));
  WriteU32(os, data.at("Loops"));
  WriteVector3(os, data.at("Position"));
  WriteVector3(os, data.at("Direction"));
  WriteVector3(os, data.at("Up"));

  const Data &extra = data.at("ExtraData");
  WriteU16(os, extra.size());
  WriteBytes(os, extra);

  if (obj_type != Presenter && obj_type != World) {
    WriteString(os, data.at("FileName"));

    WriteU32(os, data.at("Unknown26"));
    WriteU32(os, data.at("Unknown27"));
    WriteU32(os, data.at("Unknown28"));

    WriteU32(os, data.at("FileType"));

    WriteU32(os, data.at("Unknown29"));
    WriteU32(os, data.at("Unknown30"));

    if (obj_type == MxOb::Sound) {
      WriteU32(os, data.at("Unknown31"));
    }
  }
}

}
