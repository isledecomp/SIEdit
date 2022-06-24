#ifndef DATA_H
#define DATA_H

#include <fstream>

#include "types.h"

class Chunk;
class Data
{
public:
  Data();
  virtual ~Data();

  virtual void Read(std::ifstream &f, u32 sz) = 0;

};

#endif // DATA_H
