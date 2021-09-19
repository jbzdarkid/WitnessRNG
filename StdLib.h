#pragma once
#include <utility> // For move

template <typename T>
T** NewDoubleArray(int width, int height) {
  // Single allocation for the grid for perf reasons.
  T* raw = new T[width * height];
  memset(raw, 0, sizeof(T) * width * height);

  T** arr = new T*[width];
  for (int x=0; x<width; x++) arr[x] = (raw + height * x);
  return arr;
}

template <typename T>
void DeleteDoubleArray(T** arr) {
  delete arr[0]; // The grid was allocated as one contiguous region.
  delete[] arr;
}

template <typename T>
class Vector {
public:
  // Constructors, etc
  Vector() { } // Leave all values at default (empty)
  Vector(int size) {
    _data = new T[size];
    _maxPos = size;
    _pos = 0;
  }
  ~Vector() {
    if (_data != nullptr) delete[] _data;
  }
  DELETE_RO3(Vector); // Copying should be done with .Copy(), to discourage accidental copying.
  Vector(Vector&& other) noexcept { /* Move constructor */
    _data = other._data;
    _pos = other._pos;
    other._data = nullptr;
  }
  Vector& operator=(Vector&& other) noexcept { /* Move assignment */
    _data = other._data;
    _pos = other._pos;
    other._data = nullptr;
    return *this;
   }

  // Functions for range-based iteration
  T* begin() {
    return &_data[0];
  }
  const T* begin() const {
    return &_data[0];
  }
  T* end() {
    return &_data[_pos];
  }
  const T* end() const {
    return &_data[_pos];
  }

  // Functions I use
  void PushBack(const T& obj) {
    _data[_pos++] = obj;
    assert(_pos <= _maxPos);
  }

  void EmplaceBack(T&& obj) {
    _data[_pos++] = std::move(obj);
    assert(_pos <= _maxPos);
  }

  int Size() const {
    return _pos;
  }

  T operator[](int index) {
    return this->At(index);
  }
  T At(int index) {
    assert(index >= 0);
    assert(index < _pos);
    return _data[index];
  }

  T* Back() {
    return &_data[_pos];
  }
  
  T Pop() {
    assert(_pos >= 1);
    return _data[--_pos];
  }

  // Expensive functions
  void Append(const Vector<T>& other) {
    for (const T& it : other) PushBack(it);
  }

  void Append(Vector<T>&& other) {
    for (const T& it : other) EmplaceBack(it);
  }

  Vector<T> Copy() {
    Vector<T> newVector(_maxPos);
    for (const T& it : *this) newVector.PushBack(it);
    return newVector;
  }

  void Expand(int size) {
    Vector<T> newVector(_maxPos + size);
    for (const T& it : *this) newVector.PushBack(it);

    // This is a replacement for the move operator, you shouldn't move this object.
    if (_data != nullptr) free(_data);
    _data = newVector._data;
    newVector._data = nullptr;
    _maxPos = newVector._maxPos;
  }

private:
  T* _data = nullptr;
  int _maxPos = 0;
  int _pos = 0;
};
