#include "sitypes.h"

#include "chunk.h"

namespace si {

Data ReadU32(std::ifstream &is)
{
  uint32_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

Data ReadU16(std::ifstream &is)
{
  uint16_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

Data ReadU8(std::ifstream &is)
{
  uint8_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

Data ReadVector3(std::ifstream &is)
{
  Vector3 u;
  is.read((char *) &u, sizeof(u));
  return u;
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

Data ReadBytes(std::ifstream &is, size_t size)
{
  bytearray d;

  d.resize(size);
  is.read(d.data(), size);

  return d;
}

void RIFF::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Format"] = ReadU32(is);
}

void LIST::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Format"] = ReadU32(is);

  if (data["Format"] == Chunk::TYPE_MxCh) {
    data["Count"] = ReadU32(is);
  }
}

void MxSt::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
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

void MxCh::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Flags"] = ReadU16(is);
  data["Object"] = ReadU32(is);
  data["Time"] = ReadU32(is);
  data["DataSize"] = ReadU32(is);
  data["Data"] = ReadBytes(is, size - 0xE);
}

void MxOf::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  data["Count"] = ReadU32(is);
  data["Offsets"] = ReadBytes(is, size - sizeof(uint32_t));
}

void pad_::Read(std::ifstream &is, DataMap &data, uint32_t version, uint32_t size)
{
  is.seekg(size, std::ios::cur);
}

const char *MxOb::GetTypeName(Type type)
{
  switch (type) {
  case SMK:
    return "SMK";
  case WAV:
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

    if (obj_type == MxOb::WAV) {
      data["Unknown31"] = ReadU32(is);
    }
  }
}

}
