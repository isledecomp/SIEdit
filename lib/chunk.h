#ifndef CHUNK_H
#define CHUNK_H

#include <fstream>
#include <map>
#include <memory>

#include "common.h"
#include "sitypes.h"
#include "types.h"

namespace si {

class Chunk
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
    TYPE_pad_ = 0x20646170
  };

  LIBWEAVER_EXPORT Chunk();
  virtual LIBWEAVER_EXPORT ~Chunk();

  LIBWEAVER_EXPORT bool Read(const std::string &f);
  LIBWEAVER_EXPORT bool Read(const char *f);
  LIBWEAVER_EXPORT void Clear();

  typedef std::vector<Chunk*> Children;

  LIBWEAVER_EXPORT Chunk *GetParent() const { return parent_; }
  LIBWEAVER_EXPORT const Children &GetChildren() const { return children_; }
  LIBWEAVER_EXPORT void AppendChild(Chunk *chunk);
  LIBWEAVER_EXPORT bool RemoveChild(Chunk *chunk);
  LIBWEAVER_EXPORT size_t IndexOfChild(Chunk *chunk);
  LIBWEAVER_EXPORT void InsertChild(size_t index, Chunk *chunk);
  LIBWEAVER_EXPORT Chunk *RemoveChild(size_t index);
  LIBWEAVER_EXPORT Chunk *GetChildAt(size_t index) const { return children_.at(index); }
  LIBWEAVER_EXPORT size_t GetChildCount() const { return children_.size(); }

  LIBWEAVER_EXPORT Type type() const { return static_cast<Type>(id_); }
  LIBWEAVER_EXPORT const u32 &id() const { return id_; }
  LIBWEAVER_EXPORT const u32 &offset() const { return offset_; }

  LIBWEAVER_EXPORT Data &data(const std::string &key) { return data_[key]; }
  LIBWEAVER_EXPORT const Data &data(const std::string &key) const { return data_.at(key); }

  LIBWEAVER_EXPORT static const char *GetTypeDescription(Type type);
  LIBWEAVER_EXPORT const char *GetTypeDescription() const
  {
    return GetTypeDescription(type());
  }

  LIBWEAVER_EXPORT Chunk *FindChildWithType(Type type) const;
  LIBWEAVER_EXPORT Chunk *FindChildWithOffset(u32 offset) const;

private:
  // Disable copy
  Chunk(const Chunk& other);
  Chunk& operator=(const Chunk& other);

  bool Read(std::ifstream &f, u32 &version, u32 &alignment);

  static RIFF *GetReaderFromType(Type type);

  u32 id_;
  u32 offset_;
  std::map<std::string, Data> data_;

  Chunk *parent_;
  Children children_;

};

}

#endif // CHUNK_H
