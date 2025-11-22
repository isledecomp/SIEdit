#ifndef UTIL_H
#define UTIL_H

#include <fstream>
#include <iostream>

#include "types.h"

namespace si {

struct NullStream : private std::streambuf, public std::ostream {
  NullStream() : std::ostream(this) {}
  int overflow(int c) { return c; }
};

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
