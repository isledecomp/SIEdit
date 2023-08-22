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

#if defined(_WIN32)
  LIBWEAVER_EXPORT bool ReplaceWithFile(const wchar_t *f);
  LIBWEAVER_EXPORT bool ExtractToFile(const wchar_t *f) const;
#endif

  LIBWEAVER_EXPORT bool ReplaceWithFile(const char *f);
  LIBWEAVER_EXPORT bool ExtractToFile(const char *f) const;

  LIBWEAVER_EXPORT bool ReplaceWithFile(FileBase *f);
  LIBWEAVER_EXPORT bool ExtractToFile(FileBase *f) const;

  LIBWEAVER_EXPORT bytearray ExtractToMemory() const;

  LIBWEAVER_EXPORT const bytearray &GetFileHeader() const;
  LIBWEAVER_EXPORT bytearray GetFileBody() const;
  LIBWEAVER_EXPORT size_t GetFileBodySize() const;

  const MxOb::Type &type() const { return type_; }
  const MxOb::FileType &filetype() const { return filetype_; }
  const uint32_t &id() const { return id_; }
  const std::string &name() const { return name_; }
  const std::string &filename() const { return filename_; }
  const ChunkedData &data() const { return data_; }

  size_t CalculateMaximumDiskSize() const;

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
  Vector3 location_;
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
  uint32_t volume_;

  uint32_t time_offset_;

  ChunkedData data_;

private:

};

}

#endif // OBJECT_H
