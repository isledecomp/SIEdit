#ifndef MXHDDATA_H
#define MXHDDATA_H

#include "data.h"

/**
 * @brief The main header section of an Interleaf file.
 *
 * MxHd is generally the first section to appear in an Interleaf file. It contains information
 * about the file as a whole, including version information and settings for how the file should
 * be streamed.
 */
class MxHd : public Data
{
public:
  MxHd();

  virtual void Read(std::ifstream &f, u32 sz);

  /**
   * @brief The version of this Interleaf file.
   *
   * This is stored as two 16-bit words inside a 32-bit dword - the high word being the major
   * version and the low word being the minor version.
   *
   * If the version is 0x00010000, as it is in Warhammer: Shadow of the Horned Rat and early builds
   * of LEGO Island, the resulting version is 1.0. If the version is 0x00020002, as it is in the
   * retail release of LEGO Island, the resulting version number is 2.2.
   */
  u32 dwVersion;

  /**
   * @brief The amount of data to read from disk at a time
   *
   * MxStreamer will read data from the file in chunks of this size. This means all sections of the
   * file must be aligned around multiples of this number, i.e. a section starting before a multiple
   * of this number must either end before it if possible, or padding should be used to make it
   * start immediately on one. If dwBufferSize is 0x20000, sections must not cross 0x20000, 0x40000,
   * 0x60000, 0x80000, etc. If a section does, it will cause an out of bounds read as the game
   * tries to read past the buffer that it's allocated and read into.
   */
  u32 dwBufferSize;

  /**
   * @brief Buffer number
   *
   * It is currently not known exactly what this value does. Source .SS files leaked on the Korean
   * ISO call it "buffersNum", implying it's the number of buffers to read at a time (in tandem with
   * dwBufferSize), however that behavior has not been tested yet. This field also does not exist
   * on version 1.0 SI files.
   */
  u32 dwBufferCount;

};

#endif // MXHDDATA_H
