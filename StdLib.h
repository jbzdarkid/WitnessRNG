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
    _pos = 0;
    _maxPos = size;
    _data = new T[size];
  }
  ~Vector() {
    if (_data != nullptr) delete[] _data;
  }
  DELETE_RO3(Vector); // Copying should be done with .Copy(), to discourage accidental copying.
  Vector(Vector&& other) noexcept { /* Move constructor */
    _pos = other._pos;
    _maxPos = other._maxPos;
    _data = other._data;
    other._data = nullptr;
  }
  Vector& operator=(Vector&& other) noexcept { /* Move assignment */
    _pos = other._pos;
    _maxPos = other._maxPos;
    _data = other._data;
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
  void Push(const T& obj, bool expand = false) {
    if (_pos == _maxPos) {
      assert(expand);
      Expand(_maxPos + 1);
    }

    _data[_pos++] = obj;
  }

  void Emplace(T&& obj, bool expand = false) {
    if (_pos == _maxPos) {
      assert(expand);
      Expand(_maxPos + 1);
    }

    _data[_pos++] = std::move(obj);
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
  void Append(const Vector<T>& other, bool expand = false) {
    for (const T& it : other) Push(it, expand);
  }

  Vector<T> Copy() {
    Vector<T> newVector(_maxPos);
    for (const T& it : *this) newVector.Push(it);
    return newVector;
  }

  void Expand(int size) {
    Vector<T> newVector(_maxPos + size);

    // Move our data into the new vector, then claim the new vector's data as our own
    if (_data != nullptr) {
      for (T& it : *this) newVector.Emplace(std::move(it));
      delete[] _data;
    }
    _maxPos = newVector._maxPos;
    _data = newVector._data;
    newVector._data = nullptr;
  }

private:
  int _pos = 0;
  int _maxPos = 0;
  T* _data = nullptr;
};
