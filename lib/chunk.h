#ifndef CHUNK_H
#define CHUNK_H

#include <fstream>
#include <map>
#include <memory>

#include "common.h"
#include "core.h"
#include "sitypes.h"
#include "types.h"

namespace si {

class Chunk : public Core
{
public:
  enum Type
  {
    TYPE_RIFF = 0x46464952,
    TYPE_LIST = 0x5453494c,
    TYPE_MxSt = 0x7453784d,
    TYPE_MxHd = 0x6448784d,
    TYPE_MxCh = 0x6843784d,
    TYPE_MxOf = 0x664f784d,
    TYPE_MxOb = 0x624f784d,
    TYPE_MxDa = 0x6144784d,
    TYPE_pad_ = 0x20646170
  };

  LIBWEAVER_EXPORT Chunk(uint32_t id = 0);
  virtual LIBWEAVER_EXPORT ~Chunk();

  LIBWEAVER_EXPORT bool Read(const std::string &f);
  LIBWEAVER_EXPORT bool Read(const char *f);
  LIBWEAVER_EXPORT void Clear();

  LIBWEAVER_EXPORT bool Write(const std::string &f);
  LIBWEAVER_EXPORT bool Write(const char *f);

  LIBWEAVER_EXPORT Type type() const { return static_cast<Type>(id_); }
  LIBWEAVER_EXPORT const uint32_t &id() const { return id_; }
  LIBWEAVER_EXPORT const uint32_t &offset() const { return offset_; }

  LIBWEAVER_EXPORT Data &data(const std::string &key) { return data_[key]; }
  LIBWEAVER_EXPORT Data data(const std::string &key) const { return data_.at(key); }

  LIBWEAVER_EXPORT static const char *GetTypeDescription(Type type);
  LIBWEAVER_EXPORT const char *GetTypeDescription() const
  {
    return GetTypeDescription(type());
  }

  LIBWEAVER_EXPORT Chunk *FindChildWithType(Type type) const;
  LIBWEAVER_EXPORT Chunk *FindChildWithOffset(uint32_t offset) const;

private:
  bool Read(std::ifstream &f, uint32_t &version, uint32_t &alignment);
  bool Write(std::ofstream &f, uint32_t &version, uint32_t &alignment) const;

  static RIFF *GetReaderFromType(Type type);

  uint32_t id_;
  uint32_t offset_;
  DataMap data_;

};

}

#endif // CHUNK_H
