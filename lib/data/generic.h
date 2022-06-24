#ifndef GENERICDATA_H
#define GENERICDATA_H

#include "data.h"

class GenericData : public Data
{
public:
  GenericData();

  virtual void Read(std::ifstream &f, u32 sz);

  bytearray bytes;

};

#endif // GENERICDATA_H
