#ifndef MXOB_H
#define MXOB_H

#include <string>

#include "common.h"
#include "types.h"

namespace si {

class MxOb
{
public:
  enum Type
  {
    /// Smacker video
    SMK = 0x03,

    /// WAVE audio
    WAV = 0x04,

    /// World object for LegoWorldPresenter
    World = 0x06,

    /// Custom MxPresenter
    Presenter = 0x07,

    /// Event
    Event = 0x08,

    /// Animation
    Animation = 0x09,

    /// Bitmap image
    BMP = 0x0A,

    /// 3D Object
    OBJ = 0x0B,

    /// Total number of types (not a real type)
    TYPE_COUNT
  };

  static const char *GetTypeName(Type type);

  /**
   * @brief Member of MxObType enum identifying the type of object
   */
  u16 wType;

  /**
   * @brief The presenter to use (if applicable)
   *
   * If wType is set to MxOb_Presenter, this will be a string identifying which presenter to use.
   */
  char szPresenter[128];

  /// Currently unknown value
  u32 dwUnknown1;

  /**
   * @brief Name of this object
   */
  char szName[128];

  /// Currently unknown value
  u32 dwObjectID;

  /// Currently unknown value
  u32 dwUnknown3;

  /// Currently unknown value
  u32 dwUnknown4;

  /// Currently unknown value
  u32 dwUnknown5;

  /// Currently unknown value
  u32 dwUnknown6;

  /// Currently unknown value
  u32 dwUnknown7;

  /// Currently unknown value
  u32 dwUnknown8;

  /// Currently unknown value
  u32 dwUnknown9;

  /// Currently unknown value
  u32 dwUnknown10;

  /// Currently unknown value
  u32 dwUnknown11;

  /// Currently unknown value
  u32 dwUnknown12;

  /// Currently unknown value
  u32 dwUnknown13;

  /// Currently unknown value
  u32 dwUnknown14;

  /// Currently unknown value
  u32 dwUnknown15;

  /// Currently unknown value
  u32 dwUnknown16;

  /// Currently unknown value
  u32 dwUnknown17;

  /// Currently unknown value
  f32 fUnknown18;

  /// Currently unknown value
  u32 dwUnknown19;

  /// Currently unknown value
  u32 dwUnknown20;

  /// Currently unknown value
  u32 dwUnknown21;

  /// Currently unknown value
  f32 fUnknown22;

  /// Currently unknown value
  u32 dwUnknown23;

  /// Currently unknown value
  u32 dwUnknown24;

  /**
   * @brief Length of the szCreationString string
   *
   * If this is zero, szCreationString takes up no space in the file.
   */
  u16 wCreationStringLength;

  char szCreationString[128];

  /**
   * @brief Original source filename
   */
  char szFilename[128];

  /// Currently unknown value
  u32 dwUnknown26;

  /// Currently unknown value
  u32 dwUnknown27;

  /// Currently unknown value
  u32 dwUnknown28;

  /**
   * @brief 4-byte identifier for the file type
   */
  u32 dwID;

  /// Currently unknown value
  u32 dwUnknown29;

  /// Currently unknown value
  u32 dwUnknown30;

  /// Currently unknown value
  u32 dwUnknown31;

};

}

#endif // MXOB_H
