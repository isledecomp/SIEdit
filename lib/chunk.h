#ifndef CHUNK_H
#define CHUNK_H

#include <fstream>
#include <memory>

#include "bytearray.h"
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

  Chunk();
  virtual ~Chunk();

  bool Read(const std::string &f);
  bool Read(const char *f);
  void Clear();

  typedef std::vector<Chunk*> Children;

  Chunk *GetParent() const { return parent_; }
  const Children &GetChildren() const { return children_; }
  void AppendChild(Chunk *chunk);
  bool RemoveChild(Chunk *chunk);
  size_t IndexOfChild(Chunk *chunk);
  void InsertChild(size_t index, Chunk *chunk);
  Chunk *RemoveChild(size_t index);
  Chunk *GetChildAt(size_t index) const { return children_.at(index); }
  size_t GetChildCount() const { return children_.size(); }

  Type type() const { return static_cast<Type>(id_); }
  const u32 &id() const { return id_; }
  const u32 &offset() const { return offset_; }
  bytearray &data() { return data_; }
  bytearray &exdata() { return exdata_; }

  static const char *GetTypeDescription(Type type);
  const char *GetTypeDescription() const
  {
    return GetTypeDescription(type());
  }

private:
  // Disable copy
  Chunk(const Chunk& other);
  Chunk& operator=(const Chunk& other);

  bool Read(std::ifstream &f, u32 &version, u32 &alignment);

  u32 id_;
  u32 offset_;
  bytearray data_;
  bytearray exdata_;

  Chunk *parent_;
  Children children_;

};

}

#endif // CHUNK_H
