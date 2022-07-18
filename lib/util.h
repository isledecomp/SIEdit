#ifndef UTIL_H
#define UTIL_H

#include <fstream>
#include <iostream>

#include "types.h"

namespace si {

inline uint32_t ReadU32(std::istream &is)
{
  uint32_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteU32(std::ostream &os, uint32_t u)
{
  os.write((const char *) &u, sizeof(u));
}

inline uint16_t ReadU16(std::istream &is)
{
  uint16_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteU16(std::ostream &os, uint16_t u)
{
  os.write((const char *) &u, sizeof(u));
}

inline uint8_t ReadU8(std::istream &is)
{
  uint8_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteU8(std::ostream &os, uint8_t u)
{
  os.write((const char *) &u, sizeof(u));
}

inline Vector3 ReadVector3(std::istream &is)
{
  Vector3 u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteVector3(std::ostream &os, Vector3 v)
{
  os.write((const char *) &v, sizeof(v));
}

inline std::string ReadString(std::istream &is)
{
  std::string d;

  while (true) {
    char c;
    is.read(&c, 1);
    if (c == 0) {
      break;
    }
    d.push_back(c);
  }

  return d;
}

inline void WriteString(std::ostream &os, const std::string &d)
{
  os.write(d.c_str(), d.size());

  // Ensure null terminator
  const char nullterm = 0;
  os.write(&nullterm, 1);
}

inline bytearray ReadBytes(std::istream &is, size_t size)
{
  bytearray d;

  d.resize(size);
  is.read(d.data(), size);

  return d;
}

inline void WriteBytes(std::ostream &os, const bytearray &ba)
{
  os.write(ba.data(), ba.size());
}

inline std::ostream &LogDebug()
{
  return std::cout << "[DEBUG] ";
}

inline std::ostream &LogWarning()
{
  return std::cerr << "[WARNING] ";
}

inline std::ostream &LogError()
{
  return std::cerr << "[ERROR] ";
}

}

#endif // UTIL_H
