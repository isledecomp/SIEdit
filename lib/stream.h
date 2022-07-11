#ifndef STREAM_H
#define STREAM_H

#include "chunk.h"
#include "core.h"

namespace si {

class Stream : public Core
{
public:
  Stream();

  bool Parse(Chunk *chunk);

private:

};

}

#endif // STREAM_H
