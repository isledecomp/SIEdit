#include "interleaf.h"

namespace si {

Interleaf::Interleaf()
{
}

bool Interleaf::Parse(Chunk *riff)
{
  if (riff->id() != Chunk::TYPE_RIFF) {
    return false;
  }

  Chunk *hd = riff->FindChildWithType(Chunk::TYPE_MxHd);
  if (!hd) {
    return false;
  }

  version_ = hd->data("Version");
  if (version_ == 0) {
    // Unknown version
    return false;
  }

  buffer_size_ = hd->data("BufferSize");
  buffer_count_ = hd->data("BufferCount");

  Chunk *of = riff->FindChildWithType(Chunk::TYPE_MxOf);
  if (!of) {
    return false;
  }

  const Data &offset_table = of->data("Offsets");
  size_t offset_count = offset_table.size() / sizeof(uint32_t);
  DeleteChildren();
  for (size_t i=0; i<offset_count; i++) {
    Stream *o = new Stream();
    Chunk *st = riff->FindChildWithOffset(offset_table[i]);
    if (!o->Parse(st)) {
      return false;
    }
    AppendChild(o);
  }

  return true;
}

}
