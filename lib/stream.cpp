#include "stream.h"

#include "object.h"

namespace si {

Stream::Stream()
{

}

bool Stream::Parse(Chunk *chunk)
{
  if (chunk->type() != Chunk::TYPE_MxSt) {
    return false;
  }

  Chunk *obj_chunk = chunk->FindChildWithType(Chunk::TYPE_MxOb);
  if (!obj_chunk) {
    return false;
  }

  Object *obj = new Object();
  if (!obj->Parse(obj_chunk)) {
    return false;
  }

  // FIXME: Read MxCh into Objects

  return true;
}

}
