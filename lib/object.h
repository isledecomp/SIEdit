#ifndef OBJECT_H
#define OBJECT_H

#include "chunk.h"
#include "core.h"
#include "types.h"

namespace si {

class Object : public Core
{
public:
  Object();

  bool Parse(Chunk *chunk);

  bytearray &data()
  {
    return data_;
  }

private:
  MxOb::Type type_;
  std::string presenter_;
  uint32_t unknown1_;
  std::string name_;
  uint32_t id_;
  uint32_t flags_;
  uint32_t unknown4_;
  uint32_t duration_;
  uint32_t loops_;
  Vector3 position_;
  Vector3 direction_;
  Vector3 up_;
  bytearray extra_;
  std::string filename_;
  uint32_t unknown26_;
  uint32_t unknown27_;
  uint32_t unknown28_;
  uint32_t filetype_;
  uint32_t unknown29_;
  uint32_t unknown30_;
  uint32_t unknown31_;

  bytearray data_;

};

}

#endif // OBJECT_H
