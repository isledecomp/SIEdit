#ifndef INTERLEAF_H
#define INTERLEAF_H

#include "chunk.h"
#include "core.h"
#include "object.h"

namespace si {

class Interleaf : public Core
{
public:
  LIBWEAVER_EXPORT Interleaf();

  LIBWEAVER_EXPORT bool Parse(Chunk *riff);
  LIBWEAVER_EXPORT Chunk *Export() const;

private:
  bool ParseStream(Chunk *chunk);
  Chunk *ExportStream(Object *obj) const;
  Chunk *ExportMxCh(uint16_t flags, uint32_t object_id, uint32_t time, const bytearray &data = bytearray()) const;

  uint32_t version_;
  uint32_t buffer_size_;
  uint32_t buffer_count_;

};

}

#endif // INTERLEAF_H
