#include "generic.h"

GenericData::GenericData()
{
}

void GenericData::Read(std::ifstream &f, u32 sz)
{
  bytes.resize(sz);
  f.read((char*) bytes.data(), sz);
}
