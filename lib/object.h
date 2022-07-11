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
  void ProcessData(const std::vector<bytearray> &chunks);

  const MxOb::FileType &filetype() const { return filetype_; }
  const uint32_t &id() const { return id_; }
  const std::string &name() const { return name_; }
  const std::string &filename() const { return filename_; }
  bytearray &data() { return data_; }

  Object *FindSubObjectWithID(uint32_t id);

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
  MxOb::FileType filetype_;
  uint32_t unknown29_;
  uint32_t unknown30_;
  uint32_t unknown31_;

  bytearray data_;

};

}

#endif // OBJECT_H
