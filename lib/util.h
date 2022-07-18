#ifndef UTIL_H
#define UTIL_H

#include <fstream>
#include <iostream>

#include "types.h"

namespace si {

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
