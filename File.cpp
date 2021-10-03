#include "stdafx.h"
#include "File.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

using namespace std;

#define CAPACITY 1024 * 1024

File::File(const string& name) {
  _handle = (s64)CreateFileA(name.c_str(), FILE_GENERIC_READ, NULL, nullptr, OPEN_EXISTING, NULL, nullptr);
  if (_handle > 0) {
    _buffer = new Vector<u8>(CAPACITY);
    _buffer->Resize(CAPACITY); // It's a buffer, we won't ever actually fill it out.
    Read();
  }

  // If I ever want it, here's how to compute the filesize in bytes:
  // u64 fileSize;
  // u32 lowSize = GetFileSize(badFile, (DWORD*)&fileSize + 1);
  // *(u32*)&fileSize = lowSize;
}

File::~File() {
  if (_handle > 0) CloseHandle((HANDLE)_handle);
  if (_buffer) delete _buffer;
}

u8 File::Get() {
  u8 value = _buffer->At(_position);
  _position++;
  if (_position == _buffer->Size()) Read();
  return value;
}

u32 File::GetInt() {
  u8 b1 = Get();
  u8 b2 = Get();
  u8 b3 = Get();
  u8 b4 = Get();

  // Little-Endian.
  return (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
}

u8 File::Peek() {
  return _buffer->At(_position);
}

bool File::Done() {
  return _buffer == nullptr || _buffer->Empty();
}

void File::Read() {
  DWORD bytesRead;
  ReadFile((HANDLE)_handle, &_buffer->At(0), CAPACITY, &bytesRead, nullptr);
  _buffer->Resize(bytesRead);
  _position = 0;
}