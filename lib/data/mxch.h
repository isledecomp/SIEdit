#ifndef MXCH_H
#define MXCH_H

#include "common.h"
#include "types.h"

namespace si {

LIBWEAVER_PACK(struct MxCh
{
  u16 wFlags;
  u32 dwObjectParent;
  u32 dwMillisecondOffset;
  u32 dwDataSize;
});

}

#endif // MXCH_H
