#include "file.h"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#else
#include <fstream>
#define FSTR(x) static_cast<std::fstream*>(x)
#endif

namespace si {

File::File()
{
  m_Handle = NULL;
}

bool File::Open(const char *c, Mode mode)
{
#ifdef _WIN32
  m_Handle = CreateFileA(c,
                         mode == Read ? GENERIC_READ : GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         mode == Read ? OPEN_EXISTING : CREATE_NEW,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
  m_Mode = mode;
  return m_Handle != INVALID_HANDLE_VALUE;
#else
  std::ios::openmode m = std::ios::binary;

  if (mode == Read) {
    m |= std::ios::in;
  } else {
    m |= std::ios::out;
  }

  m_Handle = new std::fstream();
  FSTR(m_Handle)->open(c, m);
  if (FSTR(m_Handle)->good() && FSTR(m_Handle)->is_open()) {
    m_Mode = mode;
    return true;
  }

  return false;
#endif
}

#ifdef _WIN32
bool File::Open(const wchar_t *c, Mode mode)
{
  m_Handle = CreateFileW(c,
                         mode == Read ? GENERIC_READ : GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         mode == Read ? OPEN_EXISTING : CREATE_NEW,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
  return m_Handle != INVALID_HANDLE_VALUE;
}
#endif

File::pos_t File::pos()
{
#ifdef _WIN32
  LONG high = 0;
  DWORD low = SetFilePointer(m_Handle, 0, &high, FILE_CURRENT);
  return pos_t(high) << 32 | low;
#else
  if (m_Mode == Read) {
    return FSTR(m_Handle)->tellg();
  } else {
    return FSTR(m_Handle)->tellp();
  }
#endif
}

File::pos_t File::size()
{
#ifdef _WIN32
  DWORD high;
  DWORD low = GetFileSize(m_Handle, &high);
  return pos_t(high) << 32 | low;
#else
  pos_t before = pos();
  seek(0, SeekEnd);
  pos_t sz = pos();
  seek(before, SeekStart);
  return sz;
#endif
}

void File::seek(File::pos_t p, SeekMode s)
{
#ifdef _WIN32
  LONG high = p >> 32;
  DWORD low = (DWORD) p;

  DWORD m;
  switch (s) {
  case SeekStart:
    m = FILE_BEGIN;
    break;
  case SeekCurrent:
    m = FILE_CURRENT;
    break;
  case SeekEnd:
    m = FILE_END;
    break;
  }

  SetFilePointer(m_Handle, low, &high, m);
#else
  std::ios::seekdir d = std::ios::beg;

  switch (s) {
  case SeekStart:
    d = std::ios::beg;
    break;
  case SeekCurrent:
    d = std::ios::cur;
    break;
  case SeekEnd:
    d = std::ios::end;
    break;
  }

  if (m_Mode == Read) {
    FSTR(m_Handle)->seekg(p, d);
  } else {
    FSTR(m_Handle)->seekp(p, d);
  }
#endif
}

void File::Close()
{
#ifdef _WIN32
  CloseHandle(m_Handle);
#else
  FSTR(m_Handle)->close();
  delete FSTR(m_Handle);
  m_Handle = NULL;
#endif
}

File::pos_t File::ReadData(void *data, File::pos_t size)
{
#ifdef _WIN32
  DWORD r;
  ReadFile(m_Handle, data, (DWORD) size, &r, NULL);
  return r;
#else
  pos_t before = this->pos();
  FSTR(m_Handle)->read((char *) data, size);
  return this->pos() - before;
#endif
}

File::pos_t File::WriteData(const void *data, File::pos_t size)
{
#ifdef _WIN32
  DWORD w;
  WriteFile(m_Handle, data, (DWORD) size, &w, NULL);
  return w;
#else
  pos_t before = this->pos();
  FSTR(m_Handle)->write((const char *) data, size);
  return this->pos() - before;
#endif
}

uint8_t FileBase::ReadU8()
{
  uint8_t u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteU8(uint8_t u)
{
  WriteData(&u, sizeof(u));
}

uint16_t FileBase::ReadU16()
{
  uint16_t u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteU16(uint16_t u)
{
  WriteData(&u, sizeof(u));
}

uint32_t FileBase::ReadU32()
{
  uint32_t u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteU32(uint32_t u)
{
  WriteData(&u, sizeof(u));
}

Vector3 FileBase::ReadVector3()
{
  Vector3 u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteVector3(const Vector3 &v)
{
  WriteData(&v, sizeof(v));
}

std::string FileBase::ReadString()
{
  std::string d;

  while (true) {
    char c;
    ReadData(&c, 1);
    if (c == 0) {
      break;
    }
    d.push_back(c);
  }

  return d;
}

void FileBase::WriteString(const std::string &d)
{
  WriteData(d.c_str(), d.size());

  // Ensure null terminator
  WriteU8(0);
}

bytearray FileBase::ReadBytes(File::pos_t size)
{
  bytearray d;

  d.resize(size);
  ReadData(d.data(), size);

  return d;
}

void FileBase::WriteBytes(const bytearray &ba)
{
  WriteData(ba.data(), ba.size());
}

MemoryBuffer::MemoryBuffer()
{
  m_Position = 0;
}

MemoryBuffer::MemoryBuffer(const bytearray &data)
{
  m_Internal = data;
  m_Position = 0;
}

File::pos_t MemoryBuffer::pos()
{
  return m_Position;
}

File::pos_t MemoryBuffer::size()
{
  return m_Internal.size();
}

void MemoryBuffer::seek(File::pos_t p, SeekMode s)
{
  switch (s) {
  case SeekStart:
    m_Position = std::min(p, size());
    break;
  case SeekCurrent:
    m_Position = std::min(m_Position + p, size());
    break;
  case SeekEnd:
    if (p > size()) {
      m_Position = 0;
    } else {
      m_Position = size() - p;
    }
    break;
  }
}

File::pos_t MemoryBuffer::ReadData(void *data, File::pos_t size)
{
  pos_t remaining = m_Internal.size() - m_Position;
  size = std::min(size, remaining);
  memcpy(data, m_Internal.data() + m_Position, size);
  m_Position += size;
  return size;
}

File::pos_t MemoryBuffer::WriteData(const void *data, File::pos_t size)
{
  pos_t end = m_Position + size;
  if (end > m_Internal.size()) {
    m_Internal.resize(end);
  }
  memcpy(m_Internal.data() + m_Position, data, size);
  m_Position += size;
  return size;
}

}
