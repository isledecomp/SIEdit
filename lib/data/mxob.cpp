#include "mxob.h"

const char *si::MxOb::GetTypeName(Type type)
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
