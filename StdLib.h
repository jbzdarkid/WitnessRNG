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
  // Construct an empty vector. This allocates space in memory (or on the stack) for the vector *container*,
  // but does not allocate any memory for the vector *contents*.
  Vector() { }
  // Construct a vector of |size|. This allocates space in memory (or on the stack) for the vector *container*,
  // AND allocates memory for the vector *contents*.
  Vector(int size) {
    _size = 0;
    _capacity = size;
    _data = new T[size];
  }
  // Construct a vector from an initializer list. This allocates exactly enough space for the initializer contents.
  Vector(std::initializer_list<T> init) : Vector((int)init.size()) {
    for (T it : init) UnsafePush(it);
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

  /* Cheap functions I use */

  // Copy |value| onto the end of the vector. |T| should be a POD type.
  // WARNING: This method does not check capacity! Do not call it unless you know there is sufficient capacity.
  void UnsafePush(const T value) {
    assert(_size < _capacity);
    _data[_size++] = value;
  }

  // Copy |obj| onto the end of the vector, resizing if needed.
  void Push(const T& obj) {
    if (_size == _capacity) {
      Expand(_capacity + 1);
    }

    _data[_size++] = obj;
  }

  // Remove the last element from the vector.
  void Pop() {
    assert(_size >= 1);
    _size--;
  }

  // Remove and return the last element from the vector. Call Pop() if you do not need the return value.
  T PopValue() {
    assert(_size >= 1);
    return _data[--_size];
  }

  // Move |obj| into the end of the vector, resizing if needed.
  void Emplace(T&& obj) {
    if (_size == _capacity) {
      Expand(_capacity + 1);
    }

    _data[_size++] = std::move(obj);
  }

  // Check if the vector is empty. Note that this does not mean that it has zero capacity.
  bool Empty() const {
    return _size == 0;
  }

  // Get the vector size.
  int Size() const {
    return _size;
  }

  // Shrink (or grow) the vector's size. This does not affect the underlying data.
  void Resize(int size) {
    assert(size > 0);
    assert(size <= _capacity);
    _size = size;
  }

  // Access the vector element at |index|
  T& operator[](int index) {
    assert(index >= 0);
    assert(index < _size);
    return _data[index];
  }

  // Access the vector element at |index|
  const T& operator[](int index) const {
    assert(index >= 0);
    assert(index < _size);
    return _data[index];
  }

  // Access the vector element at |index|, just like operator[] (except this one is easier to use with pointer types).
  T& At(int index) {
    assert(index >= 0);
    assert(index < _size);
    return _data[index];
  }

  /* Expensive functions */

  // Fill the entire vector's contents with |value|. |T| should be a POD type.
  void Fill(const T value) {
    memset(_data, value, sizeof(T) * _capacity);
    _size = _capacity;
  }

  // Copy the contents of |other| into the end of this vector, resizing if needed.
  void Append(const Vector<T>& other) {
    int requiredCapacity = (_size + other._size) - _capacity;
    Expand(requiredCapacity);
    for (const T& it : other) UnsafePush(it);
  }

  // Copy the contents of the vector into a new vector with the same capacity.
  Vector<T> Copy() {
    Vector<T> newVector(_capacity);
    for (const T& it : *this) newVector.Push(it);
    return newVector;
  }

  // Resize the vector to increase its maximum capacity by |size|.
  void Expand(int size) {
    if (size <= 0) return;
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

// Compare two vector's contents. This assumes that |T| has an equality function.
// Two vectors are considered the same if their contents are equal, regardless of their respective capacities.
template <typename T>
bool operator==(const Vector<T>& a, const Vector<T>& b) {
  if (a._size != b._size) return false; // Different amounts of data => not equal.
  if (&a == &b) return true; // A and B are the same pointer (very unlikely)
  if (a._data == b._data) return true; // A and B have the same data pointers (extremely unlikely)
  for (int i=0; i<a._size; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}
