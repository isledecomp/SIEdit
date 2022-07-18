#ifndef OBJECT_H
#define OBJECT_H

#include "core.h"
#include "sitypes.h"
#include "types.h"

namespace si {

class Object : public Core
{
public:
  typedef std::vector<bytearray> ChunkedData;

  Object();

  void SetChunkedData(const ChunkedData &cd) { data_ = cd; }

  LIBWEAVER_EXPORT bytearray GetNormalizedData() const;
  LIBWEAVER_EXPORT void SetNormalizedData(const bytearray &d);

  LIBWEAVER_EXPORT static bytearray ToPackedData(MxOb::FileType filetype, const ChunkedData &chunks);
  LIBWEAVER_EXPORT static ChunkedData ToChunkedData(MxOb::FileType filetype, const bytearray &chunks);

  LIBWEAVER_EXPORT const bytearray &GetFileHeader() const;
  LIBWEAVER_EXPORT bytearray GetFileBody() const;
  LIBWEAVER_EXPORT size_t GetFileBodySize() const;

  const MxOb::Type &type() const { return type_; }
  const MxOb::FileType &filetype() const { return filetype_; }
  const uint32_t &id() const { return id_; }
  const std::string &name() const { return name_; }
  const std::string &filename() const { return filename_; }
  const ChunkedData &data() const { return data_; }

  Object *FindSubObjectWithID(uint32_t id);

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

  ChunkedData data_;

};

}

#endif // OBJECT_H
