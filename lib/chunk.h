#ifndef CHUNK_H
#define CHUNK_H

#include <fstream>
#include <memory>

#include "data/data.h"
#include "types.h"

class Chunk
{
public:
  enum Type
  {
    RIFF = 0x46464952,
    LIST = 0x5453494c,
    MxSt = 0x7453784d,
    MxHd = 0x6448784d,
    pad_ = 0x20646170
  };

  Chunk();
  virtual ~Chunk();

  bool Read(const std::string &f);
  bool Read(const char *f);
  bool Read(std::ifstream &f);
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
  Data *data() const { return data_; }

  static Data *CreateDataFromID(u32 id);

  static const char *GetTypeDescription(Type type);
  const char *GetTypeDescription() const
  {
    return GetTypeDescription(type());
  }

private:
  // Disable copy
  Chunk(const Chunk& other);
  Chunk& operator=(const Chunk& other);

  u32 id_;
  Data *data_;

  Chunk *parent_;
  Children children_;

};

#endif // CHUNK_H
