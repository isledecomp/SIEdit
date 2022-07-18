#ifndef INTERLEAF_H
#define INTERLEAF_H

#include <fstream>

#include "core.h"
#include "object.h"

namespace si {

class Interleaf : public Core
{
public:
  LIBWEAVER_EXPORT Interleaf();

  LIBWEAVER_EXPORT void Clear();

  LIBWEAVER_EXPORT bool Read(const char *f);
  LIBWEAVER_EXPORT bool Read(const wchar_t *f);

  //LIBWEAVER_EXPORT bool Write(const char *f) const;
  //LIBWEAVER_EXPORT bool Write(const wchar_t *f) const;

private:
  bool Read(std::ifstream &is);
  //bool Write(std::ofstream &os) const;

  bool ReadChunk(Core *parent, std::ifstream &is);

  Object *ReadObject(std::ifstream &is);

  uint32_t m_Version;
  uint32_t m_BufferSize;
  uint32_t m_BufferCount;

  uint32_t m_OffsetCount;
  std::vector<uint32_t> m_OffsetTable;

  std::map<uint32_t, Object*> m_ObjectIndexTable;

};

}

#endif // INTERLEAF_H
