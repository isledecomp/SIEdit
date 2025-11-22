#ifndef INFO_H
#define INFO_H

#include "core.h"

namespace si {

class Info : public Core
{
public:
  static const uint32_t NULL_OBJECT_ID = 0xFFFFFFFF;

  Info()
  {
    m_ObjectID = NULL_OBJECT_ID;
  }

  void clear()
  {
    m_Desc.clear();
    DeleteChildren();
  }

  const uint32_t &GetType() const { return m_Type; }
  void SetType(const uint32_t &t) { m_Type = t; }

  const uint32_t &GetOffset() const { return m_Offset; }
  void SetOffset(const uint32_t &t) { m_Offset = t; }

  const uint32_t &GetObjectID() const { return m_ObjectID; }
  void SetObjectID(const uint32_t &t) { m_ObjectID = t; }

  const uint32_t &GetSize() const { return m_Size; }
  void SetSize(const uint32_t &t) { m_Size = t; }

  const std::string &GetDescription() const { return m_Desc; }
  void SetDescription(const std::string &d) { m_Desc = d; }

  const bytearray &GetData() const { return m_Data; }
  void SetData(const bytearray &d) { m_Data = d; }

private:
  uint32_t m_Type;
  uint32_t m_Offset;
  uint32_t m_Size;
  uint32_t m_ObjectID;
  std::string m_Desc;
  bytearray m_Data;

};

}

#endif // INFO_H
