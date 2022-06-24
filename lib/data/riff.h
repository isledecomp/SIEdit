#ifndef RIFFDATA_H
#define RIFFDATA_H

#include "chunk.h"

class RIFFData : public Data
{
public:
  RIFFData();

  virtual void Read(std::ifstream &f, u32 sz);

  u32 id;

};

#endif // RIFFDATA_H
