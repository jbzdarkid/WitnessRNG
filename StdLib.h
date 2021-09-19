#pragma once
#include <utility> // For move
#include <initializer_list> // For initializer constructor

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
    _size = 0;
    _capacity = size;
    _data = new T[size];
  }
  Vector(std::initializer_list<T> init) : Vector((int)init.size()) {
    for (T it : init) Push(it);
  }
  ~Vector() {
    if (_data != nullptr) delete[] _data;
  }
  DELETE_RO3(Vector); // Copying should be done with .Copy(), to discourage accidental copying.
  Vector(Vector&& other) noexcept { /* Move constructor */
    _size = other._size;
    _capacity = other._capacity;
    _data = other._data;
    other._data = nullptr;
  }
  Vector& operator=(Vector&& other) noexcept { /* Move assignment */
    _size = other._size;
    _capacity = other._capacity;
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
    return &_data[_size];
  }
  const T* end() const {
    return &_data[_size];
  }

  // Functions I use
  void Push(const T& obj, bool expand = false) {
    if (_size == _capacity) {
      assert(expand);
      Expand(_capacity + 1);
    }

    _data[_size++] = obj;
  }

  void Emplace(T&& obj, bool expand = false) {
    if (_size == _capacity) {
      assert(expand);
      Expand(_capacity + 1);
    }

    _data[_size++] = std::move(obj);
  }

  bool Empty() const {
    return _size == 0;
  }
  int Size() const {
    return _size;
  }

  T& operator[](int index) {
    assert(index >= 0);
    assert(index < _size);
    return _data[index];
  }
  const T& operator[](int index) const {
    assert(index >= 0);
    assert(index < _size);
    return _data[index];
  }
  T& At(int index) {
    assert(index >= 0);
    assert(index < _size);
    return _data[index];
  }

  T* Back() {
    return &_data[_size];
  }
  
  T Pop() {
    assert(_size >= 1);
    return _data[--_size];
  }

  // Expensive functions
  void Append(const Vector<T>& other, bool expand = false) {
    for (const T& it : other) Push(it, expand);
  }

  Vector<T> Copy() {
    Vector<T> newVector(_capacity);
    for (const T& it : *this) newVector.Push(it);
    return newVector;
  }

  void Expand(int size) {
    Vector<T> newVector(_capacity + size);

    // Move our data into the new vector, then claim the new vector's data as our own
    if (_data != nullptr) {
      for (T& it : *this) newVector.Emplace(std::move(it));
      delete[] _data;
    }
    _capacity = newVector._capacity;
    _data = newVector._data;
    newVector._data = nullptr;
  }

private:
  template <typename T>
  friend bool operator==(const Vector<T>& a, const Vector<T>& b);

  int _size = 0;
  int _capacity = 0;
  T* _data = nullptr;
};

template <typename T>
[[deprecated]]
bool operator==(const Vector<T>& a, const Vector<T>& b) {
  if (a._size != b._size) return false; // Note that they may have different capacities.
  if (&a == &b) return true; // Very unlikely.
  if (a._data == b._data) return true; // Very unlikely.
  for (int i=0; i<a._size; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}
