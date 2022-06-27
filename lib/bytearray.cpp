#include "bytearray.h"

#include <iomanip>
#include <ios>
#include <sstream>

namespace si {

bytearray::bytearray()
{

}

std::string bytearray::hex() const
{
  std::ostringstream ss;

  ss << std::hex << std::uppercase << std::setfill('0');

  for (size_t i=0; i<size(); i++) {
    ss << std::setw(2) << at(i);
  }

  return ss.str();
}

}
