#ifndef FILE_H
#define FILE_H

#include <fstream>

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

  uint8_t ReadU8();
  uint16_t ReadU16();
  uint32_t ReadU32();
  std::string ReadString();
  bytearray ReadBytes(size_t size);
  Vector3 ReadVector3();
  virtual size_t ReadData(void *data, size_t size) = 0;

  void WriteU8(uint8_t u);
  void WriteU16(uint16_t u);
  void WriteU32(uint32_t u);
  void WriteString(const std::string &s);
  void WriteBytes(const bytearray &b);
  void WriteVector3(const Vector3 &b);
  virtual size_t WriteData(const void *data, size_t size) = 0;

  virtual void Close() {}

  enum SeekMode
  {
    SeekStart,
    SeekCurrent,
    SeekEnd
  };

  virtual size_t pos() = 0;
  virtual size_t size() = 0;
  virtual void seek(size_t p, SeekMode s = SeekStart) = 0;
  virtual bool atEnd() = 0;

};

class File : public FileBase
{
public:
  virtual ~File()
  {
    Close();
  }

  bool Open(const char *c, Mode mode);

#ifdef _WIN32
  bool Open(const wchar_t *c, Mode mode);
#endif

  virtual size_t pos();
  virtual size_t size();
  virtual void seek(size_t p, SeekMode s = SeekStart);
  virtual bool atEnd();

  virtual void Close();
  virtual size_t ReadData(void *data, size_t size);
  virtual size_t WriteData(const void *data, size_t size);

private:
  std::fstream m_Stream;
  Mode m_Mode;

};

class MemoryBuffer : public FileBase
{
public:
  LIBWEAVER_EXPORT MemoryBuffer();
  LIBWEAVER_EXPORT MemoryBuffer(const bytearray &data);

  LIBWEAVER_EXPORT virtual size_t pos();
  LIBWEAVER_EXPORT virtual size_t size();
  LIBWEAVER_EXPORT virtual void seek(size_t p, SeekMode s = SeekStart);
  LIBWEAVER_EXPORT virtual bool atEnd();

  const bytearray &data() const { return m_Internal; }

  LIBWEAVER_EXPORT virtual size_t ReadData(void *data, size_t size);
  LIBWEAVER_EXPORT virtual size_t WriteData(const void *data, size_t size);

private:
  bytearray m_Internal;
  size_t m_Position;

};

}

#endif // FILE_H
