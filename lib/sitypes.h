#ifndef SI_H
#define SI_H

#include <fstream>

#include "file.h"
#include "types.h"

namespace si {

/**
 * @brief RIFF chunk type
 *
 * Name     | Size     | Type     | Description
 * -------- | -------- | -------- | -----------
 * Format   | 4        | u32      | 4-byte ASCII identifier for what type of RIFF this is (usually 'OMNI' in the case of LEGO Island)
 */
class RIFF
{
public:
  enum Type {
    RIFF_ = 0x46464952,
    LIST = 0x5453494c,
    MxSt = 0x7453784d,
    MxHd = 0x6448784d,
    MxCh = 0x6843784d,
    MxOf = 0x664f784d,
    MxOb = 0x624f784d,
    MxDa = 0x6144784d,
    pad_ = 0x20646170,
    OMNI = 0x494e4d4f,
    WAVE = 0x45564157,
    fmt_ = 0x20746D66,
    data = 0x61746164
  };

  struct Chk
  {
    size_t size_position;
    size_t data_start;
  };

  static Chk BeginChunk(FileBase *f, uint32_t type);
  static void EndChunk(FileBase *f, const Chk &stat);

  static inline std::string PrintU32AsString(uint32_t u)
  {
    return std::string((const char *) &u, sizeof(u));
  }

  LIBWEAVER_EXPORT static const char *GetTypeDescription(Type t);

};

/**
 * @brief LIST chunk type
 *
 * Name     | Size     | Type     | Description
 * -------- | -------- | -------- | -----------
 * Format   | 4        | u32      | 4-byte ASCII identifier for what type of LIST this is.
 * Count    | 4        | u32      | (Optional) for 'MxCh' type LISTs, the number of elements in this list.
 */
class LIST : public RIFF
{
public:
  enum Variation {
    Act_ = 0x00746341,
    RAND = 0x444e4152
  };
};

/**
 * @brief MxHd chunk type
 *
 * Name        | Size     | Type     | Description
 * ----------- | -------- | -------- | -----------
 * Version     | 4        | u32      | Version of this SI file stored as two packed 16-bit words, the high word being the major version and the low word being the minor version.
 * BufferSize  | 4        | u32      | The amount of data to read from disk at a time.
 * BufferCount | 4        | u32      | FIXME: Currently not understood what this field does.
 */
class MxHd : public RIFF
{
public:
};

/**
 * @brief MxSt chunk type
 *
 * MxSt is a container type only, it has none of its own members.
 */
class MxSt : public RIFF
{
public:
};

/**
 * @brief MxCh chunk type
 *
 * Name       | Size     | Type      | Description
 * ---------- | -------- | --------- | -----------
 * Flags      | 2        | u16       | Flags determining the behavior of this chunk.
 * Object     | 4        | u32       | ID of the MxOb that this chunk belongs to.
 * Time       | 4        | u32       | Time in milliseconds that this chunk's data should be presented at.
 * DataSize   | 4        | u32       | Size of the data in this chunk.
 * Data       | DataSize | bytearray | Actual data in chunk.
 */
class MxCh : public RIFF
{
public:
  enum Flag {
    FLAG_SPLIT = 0x10,
    FLAG_END = 0x2
  };

  static const uint32_t HEADER_SIZE = 14;
};

/**
 * @brief MxOf chunk type
 *
 * Name       | Size     | Type             | Description
 * ---------- | -------- | ---------------- | -----------
 * Count      | 4        | u32              | Number of objects in this list. Not necessarily the number of offsets, as one offset may point to an object with multiple sub-objects.
 * Offsets    | Variable | bytearray/u32[]  | List of 4-byte file offsets where objects begin.
 */
class MxOf : public RIFF
{
public:
};

/**
 * @brief pad_ chunk type
 *
 * Denotes padding to optimize disc reads. Contains no useful information,
 * customarily filled with the byte 0xCD.
 */
class pad_ : public RIFF
{
public:
};

/**
 * @brief MxOb chunk type
 *
 * Name        | Size     | Type             | Description
 * ----------- | -------- | ---------------- | -----------
 * Type        | 2        | u16              | Type of object (member of MxOb::Type enum)
 * Presenter   | Variable | string           | Null-terminated string identifying the presenter to use (if type is set to `Presenter`)
 * Unknown1    | 4        | u32              |
 * Name        | Variable | string           | Null-terminated string identifying object's name
 * ID          | 4        | u32              | Unique object identifier within file (used to differentiate interleaved MxChs)
 * Flags       | 4        | u32              | Flags of object (member of MxOb::Flags enum)
 * Unknown4    | 4        | u32              | Similar to Duration, but only used for Lego3DWavePresenter
 * Duration    | 4        | u32              | Duration in milliseconds * Loops
 * Loops       | 4        | u32              |
 * Position    | 24       | Vector3          | Position
 * Direction   | 24       | Vector3          | Direction to look towards
 * Up          | 24       | Vector3          | Up vector
 * ExtraLength | 2        | u16              |
 * ExtraData   | ExtraLength | bytearray     |
 * FileName    | Variable | string           | Original filename of the file represented by this object.
 * Unknown26   | 4        | u32              |
 * Unknown27   | 4        | u32              |
 * Unknown28   | 4        | u32              |
 * FileType    | 4        | u32              | 4-byte ASCII ID for the file type
 * Unknown29   | 4        | u32              |
 * Unknown30   | 4        | u32              |
 * Volume      | 4        | u32              | Only populated for WAV files. Audio volume from 0 - 79.
 */
class MxOb : public RIFF
{
public:
  enum Type
  {
    /// Not an MxOb type, this is our identifier for an object that is in the TOC but doesn't
    /// actually exist
    Null = -1,

    /// Video
    Video = 0x03,

    /// Audio
    Sound = 0x04,

    /// World object for LegoWorldPresenter
    World = 0x06,

    /// Custom MxPresenter
    Presenter = 0x07,

    /// Event
    Event = 0x08,

    /// Animation
    Animation = 0x09,

    /// Bitmap image
    Bitmap = 0x0A,

    /// 3D Object
    Object = 0x0B,

    /// Total number of types (not a real type)
    TYPE_COUNT
  };

  enum Flags
  {
    /// Object loops via cache (i.e. hard disk)
    LoopCache = 0x01,

    /// Object does not loop
    NoLoop = 0x02,

    /// Object loops via stream (i.e. CD-ROM)
    LoopStream = 0x04,

    /// Object is transparent
    Transparent = 0x08,

    /// Unknown flag, but set by every object thus far
    Unknown = 0x20,

    /// Total number of flags (not a real type)
    FLAGS_COUNT,
  };

  enum FileType
  {
    /// WAVE audio
    WAV = 0x56415720,

    /// Bitmap image
    STL = 0x4C545320,

    /// FLIC animation
    FLC = 0x434c4620,

    /// SMK video
    SMK = 0x4b4d5320,

    /// 3D Object
    OBJ = 0x4a424f20,
  };

  // FIXME: sitypes.h probably won't be part of the public API, so this should
  //        probably be moved
  LIBWEAVER_EXPORT static const char *GetTypeName(Type type);
  LIBWEAVER_EXPORT static std::vector<const char*> GetFlagsName(Flags flags);

  static const int MAXIMUM_VOLUME = 79;

};

}

#endif // SI_H
