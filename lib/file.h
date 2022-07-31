#ifndef FILE_H
#define FILE_H

#include "types.h"

namespace si {

class FileBase
{
public:
  FileBase()
  {
  }

  virtual ~FileBase()
  {
  }

  enum Mode {
    Read,
    Write
  };

  typedef uint64_t pos_t;

  uint8_t ReadU8();
  uint16_t ReadU16();
  uint32_t ReadU32();
  std::string ReadString();
  bytearray ReadBytes(pos_t size);
  Vector3 ReadVector3();
  virtual pos_t ReadData(void *data, pos_t size) = 0;

  void WriteU8(uint8_t u);
  void WriteU16(uint16_t u);
  void WriteU32(uint32_t u);
  void WriteString(const std::string &s);
  void WriteBytes(const bytearray &b);
  void WriteVector3(const Vector3 &b);
  virtual pos_t WriteData(const void *data, pos_t size) = 0;

  virtual void Close() {}

  enum SeekMode
  {
    SeekStart,
    SeekCurrent,
    SeekEnd
  };

  virtual pos_t pos() = 0;
  virtual pos_t size() = 0;
  virtual void seek(pos_t p, SeekMode s = SeekStart) = 0;
  LIBWEAVER_EXPORT bool atEnd() { return pos() == size(); }

};

class File : public FileBase
{
public:
  File();

  virtual ~File()
  {
    Close();
  }

  bool Open(const char *c, Mode mode);

#ifdef _WIN32
  bool Open(const wchar_t *c, Mode mode);
#endif

  virtual pos_t pos();
  virtual pos_t size();
  virtual void seek(pos_t p, SeekMode s = SeekStart);

  virtual void Close();
  virtual pos_t ReadData(void *data, pos_t size);
  virtual pos_t WriteData(const void *data, pos_t size);

private:
  void *m_Handle;
  Mode m_Mode;

};

class MemoryBuffer : public FileBase
{
public:
  LIBWEAVER_EXPORT MemoryBuffer();
  LIBWEAVER_EXPORT MemoryBuffer(const bytearray &data);

  LIBWEAVER_EXPORT virtual pos_t pos();
  LIBWEAVER_EXPORT virtual pos_t size();
  LIBWEAVER_EXPORT virtual void seek(pos_t p, SeekMode s = SeekStart);

  const bytearray &data() const { return m_Internal; }

  LIBWEAVER_EXPORT virtual pos_t ReadData(void *data, pos_t size);
  LIBWEAVER_EXPORT virtual pos_t WriteData(const void *data, pos_t size);

private:
  bytearray m_Internal;
  pos_t m_Position;

};

}

#endif // FILE_H
