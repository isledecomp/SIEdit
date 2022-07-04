#ifndef TYPES_H
#define TYPES_H

#include <map>
#include <string>
#include <vector>

namespace si {

typedef unsigned char       u8;
typedef char                s8;
typedef unsigned short      u16;
typedef short               s16;
typedef unsigned int        u32;
typedef int                 s32;
typedef float               f32;
typedef double              f64;

class bytearray : public std::vector<s8>
{
public:
  bytearray() = default;

  template <typename T>
  T *cast() { return reinterpret_cast<T*>(data()); }

  template <typename T>
  const T *cast() const { return reinterpret_cast<const T*>(data()); }

};

class Vector3
{
public:
  Vector3(){}
  Vector3(f64 ix, f64 iy, f64 iz)
  {
    x = ix;
    y = iy;
    z = iz;
  }

  f64 x;
  f64 y;
  f64 z;
};

class Data
{
public:
  inline Data()
  {
    data_.resize(sizeof(si::u32));
    //memset(data_.data(), 0, data_.size());
  }

  inline Data(const u32 &u) { set(u); }
  inline Data(const Vector3 &u) { set(u); }
  inline Data(const bytearray &u) { set(u); }
  inline Data(const std::string &u)
  {
    data_.resize(u.size());
    memcpy(data_.data(), u.data(), u.size());
  }

  inline operator u32() const
  {
    return toU32();
  }

  inline operator const char *() const
  {
    return data();
  }

  inline operator Vector3() const
  {
    return toVector3();
  }

  inline u16 toU16() const { return *data_.cast<si::u16>(); }
  inline s16 toS16() const { return *data_.cast<si::s16>(); }
  inline u32 toU32() const { return *data_.cast<si::u32>(); }
  inline s32 toS32() const { return *data_.cast<si::s32>(); }
  inline Vector3 toVector3() const { return *data_.cast<si::Vector3>(); }
  inline const char *data() const { return data_.data(); };
  inline char *data() { return data_.data(); };
  inline const char *c_str() const { return this->data(); };
  inline size_t size() const { return data_.size(); }
  inline const std::string toString() const
  {
    // Subtract 1 from size, assuming the last character is a null terminator
    return std::string(data_.data(), std::max(size_t(0), data_.size()-1));
  }

  inline bool operator==(int u) const
  {
    return get<int>() == u;
  }

  inline bool operator==(si::u32 u) const
  {
    return get<si::u32>() == u;
  }

  template <typename T>
  inline const T &get() const
  {
    return *data_.cast<T>();
  }

  template <typename T>
  inline void set(const T &value)
  {
    data_.resize(sizeof(T));
    memcpy(data_.data(), &value, sizeof(T));
  }

  inline void set(const bytearray &value) { data_ = value; }

private:
  bytearray data_;

};

using DataMap = std::map<std::string, Data>;

}

#endif // TYPES_H
