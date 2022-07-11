#include "object.h"

namespace si {

Object::Object()
{

}

bool Object::Parse(Chunk *chunk)
{
  type_ = static_cast<MxOb::Type>(chunk->data("Type").toU16());
  presenter_ = chunk->data("Presenter").toString();
  unknown1_ = chunk->data("Unknown1");
  name_ = chunk->data("Name").toString();
  id_ = chunk->data("ID");
  flags_ = chunk->data("Flags");
  unknown4_ = chunk->data("Unknown4");
  duration_ = chunk->data("Duration");
  loops_ = chunk->data("Loops");
  position_ = chunk->data("Position");
  direction_ = chunk->data("Direction");
  up_ = chunk->data("Up");
  extra_ = chunk->data("ExtraData");
  filename_ = chunk->data("FileName").toString();
  unknown26_ = chunk->data("Unknown26");
  unknown27_ = chunk->data("Unknown27");
  unknown28_ = chunk->data("Unknown28");
  filetype_ = chunk->data("FileType");
  unknown29_ = chunk->data("Unknown29");
  unknown30_ = chunk->data("Unknown30");
  unknown31_ = chunk->data("Unknown31");

  return true;
}

}
