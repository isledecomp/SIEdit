#include "mxhd.h"

MxHd::MxHd()
{

}

void MxHd::Read(std::ifstream &f, u32 sz)
{
  // Read version information
  f.read((char *) &dwVersion, sizeof(dwVersion));

  if (dwVersion == 0x00010000) { // 1.0
    // This MxHd is only 8 bytes long
    // FIXME: Not actually sure yet what the second 4 bytes are for, so we just read them into here
    f.read((char *) &dwBufferSize, sizeof(dwBufferSize));
  } else {
    // Version 2.2 has buffer size and buffer count, known due to the leaked .SS files on Korean
    f.read((char *) &dwBufferSize, sizeof(dwBufferSize));
    f.read((char *) &dwBufferCount, sizeof(dwBufferCount));
  }
}
