#ifndef TYPES_H
#define TYPES_H

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <streambuf>
#include <string>
#include <ostream>
#include <vector>

#if defined(__GNUC__)
#define LIBWEAVER_PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#define LIBWEAVER_PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#ifdef _MSC_VER
#define LIBWEAVER_EXPORT __declspec(dllexport)
#else
#define LIBWEAVER_EXPORT
#endif

#if defined(_WIN32)
#define LIBWEAVER_OS_WINDOWS
#elif defined(__APPLE__)
#define LIBWEAVER_OS_MACOS
#elif defined(__linux__)
#define LIBWEAVER_OS_LINUX
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1600)
// Declare types for MSVC versions less than 2010 (1600) which lacked a stdint.h
typedef unsigned char       uint8_t;
typedef char                int8_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
#else
#include <stdint.h>
#endif

namespace si {

class bytearray : public std::vector<char>
{
public:
  bytearray(){}
  bytearray(size_t size)
  {
    resize(size);
  }
  bytearray(const char *data, size_t size)
  {
    resize(size);
    memcpy(this->data(), data, size);
  }

  template <typename T>
  T *cast() { return reinterpret_cast<T*>(data()); }

  template <typename T>
  const T *cast() const { return reinterpret_cast<const T*>(data()); }

  void append(const char *data, size_t size)
  {
    size_t current = this->size();
    this->resize(current + size);
    memcpy(this->data() + current, data, size);
  }

  void append(const bytearray &other)
  {
    size_t current = this->size();
    this->resize(current + other.size());
    memcpy(this->data() + current, other.data(), other.size());
  }

  void fill(char c)
  {
    memset(this->data(), c, this->size());
  }

  bytearray left(size_t sz) const
  {
    bytearray b(std::min(sz, this->size()));
    memcpy(b.data(), this->data(), b.size());
    return b;
  }

  bytearray mid(size_t i, size_t size = 0) const
  {
    if (i >= this->size()) {
      return bytearray();
    }

    size_t target = this->size() - i;
    if (size != 0) {
      target = std::min(target, size);
    }

    bytearray b(target);
    memcpy(b.data(), this->data() + i, b.size());
    return b;
  }

  bytearray right(size_t i) const
  {
    if (i >= size()) {
      return *this;
    }

    bytearray b(i);
    memcpy(b.data(), this->data() + this->size() - i, b.size());
    return b;
  }

};

class Vector3
{
public:
  Vector3(){}
  Vector3(double ix, double iy, double iz)
  {
    x = ix;
    y = iy;
    z = iz;
  }

  double x;
  double y;
  double z;
};

class Data
{
public:
  inline Data()
  {
    data_.resize(sizeof(uint32_t));
    memset(data_.data(), 0, data_.size());
  }

  inline Data(const uint32_t &u) { set(u); }
  inline Data(const Vector3 &u) { set(u); }
  inline Data(const bytearray &u) { set(u); }
  inline Data(const std::string &u)
  {
    data_.resize(u.size());
    memcpy(data_.data(), u.data(), u.size());
  }

  inline operator uint32_t() const { return toU32(); }
  inline operator const char *() const { return data(); }
  inline operator Vector3() const { return toVector3(); }
  inline operator bytearray() const { return data_; }
  inline operator std::string() const { return toString(); }

  inline uint16_t toU16() const { return *data_.cast<uint16_t>(); }
  inline int16_t toS16() const { return *data_.cast<int16_t>(); }
  inline uint32_t toU32() const { return *data_.cast<uint32_t>(); }
  inline int32_t toS32() const { return *data_.cast<int32_t>(); }
  inline Vector3 toVector3() const { return *data_.cast<Vector3>(); }
  inline const char *data() const { return data_.data(); };
  inline char *data() { return data_.data(); };
  inline const char *c_str() const { return this->data(); };
  inline size_t size() const { return data_.size(); }
  inline const std::string toString() const
  {
    if (data_.empty()) {
      return std::string();
    } else {
      // Subtract 1 from size, assuming the last character is a null terminator
      return std::string(data_.data(), std::max(size_t(0), data_.size()-1));
    }
  }

  inline bool operator==(int u) const
  {
    return get<int>() == u;
  }

  inline bool operator==(uint32_t u) const
  {
    return get<uint32_t>() == u;
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

}

#endif // TYPES_H
