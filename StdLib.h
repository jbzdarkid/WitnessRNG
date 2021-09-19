#pragma once
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
  Vector(int size) {
    _data = (T*)malloc(sizeof(T) * size);
    _maxPos = size;
    _pos = 0;
  }
  ~Vector() {
    if (_data != nullptr) free(_data);
  }
  Vector(const Vector& other) = delete; /* Copy constructor */
  Vector& operator=(const Vector& other) = delete; /* Copy assignment */
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
    EmplaceBack(obj);
  }

  void EmplaceBack(T&& obj) {
    _data[_pos++] = move(obj);
    assert(_pos < _maxPos);
  }

  int Size() {
    return _pos;
  }

  T operator[](int index) {
    return this->At(index);
  }
  T At(int index) {
    assert(index >= 0);
    assert(index < _pos);
    return _data[_pos];
  }

private:
  T* _data = nullptr;
  int _maxPos = 0;
  int _pos = 0;
};
