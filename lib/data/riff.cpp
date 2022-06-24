#include "riff.h"

RIFFData::RIFFData()
{

}

void RIFFData::Read(std::ifstream &f, u32 sz)
{
  f.read((char *) &id, sizeof(id));
}
