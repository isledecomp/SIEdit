#ifndef INTERLEAF_H
#define INTERLEAF_H

#include <fstream>

#include "core.h"
#include "file.h"
#include "info.h"
#include "object.h"

namespace si {

class Interleaf : public Core
{
public:
  enum Error
  {
    ERROR_SUCCESS,
    ERROR_IO,
    ERROR_INVALID_INPUT,
    ERROR_INVALID_BUFFER_SIZE
  };

  enum Version
  {
    Version2_1 = 0x00010002,
    Version2_2 = 0x00020002
  };

  LIBWEAVER_EXPORT Interleaf();

  LIBWEAVER_EXPORT void Clear();

  LIBWEAVER_EXPORT Error Read(const char *f);
  LIBWEAVER_EXPORT Error Write(const char *f) const;

#ifdef _WIN32
  LIBWEAVER_EXPORT Error Read(const wchar_t *f);
  LIBWEAVER_EXPORT Error Write(const wchar_t *f) const;
#endif

  Error Read(FileBase *is);
  Error Write(FileBase *os) const;

  Info *GetInformation() { return &m_Info; }

private:
  Error ReadChunk(Core *parent, FileBase *f, Info *info);

  Object *ReadObject(FileBase *f, Object *o, std::stringstream &desc);
  void WriteObject(FileBase *f, const Object *o) const;

  void InterleaveObjects(FileBase *f, const std::vector<Object*> &objects) const;

  void WriteSubChunk(FileBase *f, uint16_t flags, uint32_t object, uint32_t time, const bytearray &data = bytearray()) const;
  void WriteSubChunkInternal(FileBase *f, uint16_t flags, uint32_t object, uint32_t time, uint32_t data_sz, const bytearray &data) const;

  void WritePadding(FileBase *f, uint32_t size) const;
  void WritePaddingIfNecessary(FileBase *f, size_t projectedWrite) const;

  Info m_Info;

  uint32_t m_Version;
  uint32_t m_BufferSize;
  uint32_t m_BufferCount;

  std::vector<uint32_t> m_ObjectList;
  std::map<uint32_t, Object*> m_ObjectIDTable;

  uint32_t m_JoiningProgress;
  uint32_t m_JoiningSize;

};

}

#endif // INTERLEAF_H
