#pragma once
#include "forward.h"
#include <string>

class File {
public:
  File(const std::string& name);
  ~File();
  DELETE_RO3(File);

  u8 Get();
  u32 GetInt();
  u8 Peek(u8 index = 0);
  bool Done();

private:
  // Reads the next CAPACITY bytes into the buffer.
  void Read();
  // Expands the buffer, then reads the next CAPACITY bytes.
  void Fetch();

  s64 _handle = 0;
  Vector<u8>* _buffer = nullptr;
  int _position = 0;
};
