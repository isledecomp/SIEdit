#ifndef __LIBWEAVER_UTIL_H
#define __LIBWEAVER_UTIL_H

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

#endif // __LIBWEAVER_UTIL_H
