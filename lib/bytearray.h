#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <string>

#include "types.h"

namespace si {

class bytearray : public std::vector<s8>
{
public:
  bytearray();

  template <typename T>
  T *cast() { return reinterpret_cast<T*>(data()); }

  std::string hex() const;

};

}

#endif // BYTEARRAY_H
