#include "sitypes.h"

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

}
