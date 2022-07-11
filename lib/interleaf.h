#ifndef INTERLEAF_H
#define INTERLEAF_H

#include "chunk.h"
#include "core.h"
#include "stream.h"

namespace si {

class Interleaf : public Core
{
public:
  LIBWEAVER_EXPORT Interleaf();

  LIBWEAVER_EXPORT bool Parse(Chunk *riff);

private:
  uint32_t version_;
  uint32_t buffer_size_;
  uint32_t buffer_count_;

};

}

#endif // INTERLEAF_H
