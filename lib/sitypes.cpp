#include "sitypes.h"

#include "util.h"

namespace si {

const char *MxOb::GetTypeName(Type type)
{
  switch (type) {
  case Video:
    return "SMK";
  case Sound:
    return "WAV";
  case Presenter:
    return "MxPresenter";
  case Bitmap:
    return "BMP";
  case Object:
    return "3D Object";
  case World:
    return "World";
  case Event:
    return "Event";
  case Animation:
    return "Animation";
  case Null:
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

const char *RIFF::GetTypeDescription(Type t)
{
  switch (t) {
  case RIFF_:
    return "Resource Interchange File Format";
  case LIST:
    return "List of sub-elements";
  case MxSt:
    return "Stream";
  case MxHd:
    return "Interleaf Header";
  case MxCh:
    return "Data Chunk";
  case MxOf:
    return "Offset Table";
  case pad_:
    return "Padding";
  case MxOb:
    return "Streamable Object";
  case MxDa:
    return "Data";
  case WAVE:
    return "WAVE";
  case fmt_:
    return "WAVE Format";
  case OMNI:
    return "OMNI";
  case data:
    return "WAVE Data";
  }

  return "Unknown";
}

RIFF::Chk RIFF::BeginChunk(std::ostream &os, uint32_t type)
{
  Chk stat;

  WriteU32(os, type);
  stat.size_position = os.tellp();
  WriteU32(os, 0);
  stat.data_start = os.tellp();

  return stat;
}

void RIFF::EndChunk(std::ostream &os, const Chk &stat)
{
  std::ios::pos_type now = os.tellp();

  uint32_t sz = now - stat.data_start;

  os.seekp(stat.size_position);
  WriteU32(os, sz);

  os.seekp(now);

  if (sz%2 == 1) {
    char nullterm = 0;
    os.write(&nullterm, 1);
  }
}

}
