#ifndef MXSTDATA_H
#define MXSTDATA_H

#include "data.h"

class MxStData : public Data
{
public:
  MxStData();

  virtual void Read(std::ifstream &f, u32 sz);

};

#endif // MXSTDATA_H
