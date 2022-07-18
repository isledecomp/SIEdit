#ifndef UTIL_H
#define UTIL_H

#include <fstream>

#include "types.h"

namespace si {

inline uint32_t ReadU32(std::ifstream &is)
{
  uint32_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteU32(std::ofstream &os, uint32_t u)
{
  os.write((const char *) &u, sizeof(u));
}

inline uint16_t ReadU16(std::ifstream &is)
{
  uint16_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteU16(std::ofstream &os, uint16_t u)
{
  os.write((const char *) &u, sizeof(u));
}

inline uint8_t ReadU8(std::ifstream &is)
{
  uint8_t u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteU8(std::ofstream &os, uint8_t u)
{
  os.write((const char *) &u, sizeof(u));
}

inline Vector3 ReadVector3(std::ifstream &is)
{
  Vector3 u;
  is.read((char *) &u, sizeof(u));
  return u;
}

inline void WriteVector3(std::ofstream &os, Vector3 v)
{
  os.write((const char *) &v, sizeof(v));
}

inline std::string ReadString(std::ifstream &is)
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

inline void WriteString(std::ofstream &os, const std::string &d)
{
  os.write(d.c_str(), d.size());

  // Ensure null terminator
  const char nullterm = 0;
  os.write(&nullterm, 1);
}

inline bytearray ReadBytes(std::ifstream &is, size_t size)
{
  bytearray d;

  d.resize(size);
  is.read(d.data(), size);

  return d;
}

inline void WriteBytes(std::ofstream &os, const bytearray &ba)
{
  os.write(ba.data(), ba.size());
}

}

#endif // UTIL_H
