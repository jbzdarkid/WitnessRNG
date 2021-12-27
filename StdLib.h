#pragma once
#include <utility> // For move
#include <initializer_list> // For initializer constructor
#include <cstring> // For memset, memcpy

#ifndef assert
#ifdef _DEBUG
#define assert(cond) \
{ \
  if (!(cond)) { \
    *static_cast<volatile int*>(nullptr) = 1; \
  } \
}
#else
#define assert(cond)
#endif
#endif

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
T** NewDoubleArray2(int width, int height) {
  // Single allocation for the grid for perf reasons.
  T* raw = (T*)malloc(sizeof(T) * width * height);
  if (raw == nullptr) return nullptr;
  memset(raw, 0, sizeof(T) * width * height);

  T** arr = new T*[width];
  for (int x=0; x<width; x++) arr[x] = (raw + height * x);
  return arr;
}

template <typename T>
void DeleteDoubleArray2(T** arr) {
  free(arr[0]); // The grid was allocated as one contiguous region.
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
    for (const T& it : init) UnsafePush(it);
  }
  ~Vector() {
    if (_data != nullptr) delete[] _data;
  }

  // Copying should be done with .Copy(), to discourage accidental copying.
  Vector(const Vector& other) = delete; /* Copy constructor */
  Vector& operator=(const Vector& other) = delete; /* Copy assignment */

  // Moving is OK, we need it for when we have a Vector<Vector<>>
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

  // Copy |value| onto the end of the vector. |T| should be a POD type to avoid unnecessary copying.
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

  // Get the vector's size, aka the number of initialized elements.
  int Size() const {
    return _size;
  }

  // Shrink (or grow) the vector's size to equal |size|. This does not affect the underlying data nor capacity.
  void Resize(int size) {
    assert(size >= 0);
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
  Vector<T> Copy() const {
    Vector<T> newVector(_capacity);
    for (const T& it : *this) newVector.Push(it);
    return newVector;
  }

  // Copy the contents of the vector into |dest|, a raw array containing |destSize| bytes
  void CopyIntoArray(T* dest, size_t destSize) const {
    size_t sizeInBytes = sizeof(T) * _size;
    assert(destSize == sizeInBytes);
    int err = memcpy_s(dest, destSize, _data, sizeInBytes);
    assert(err == 0);
  }

  // Set the contents of this vector from |dest|, a raw array containing |destSize| bytes
  void CopyFromArray(const T* src, size_t srcSize) {
    size_t sizeInBytes = sizeof(T) * _size;
    assert(srcSize == sizeInBytes);
    int err = memcpy_s(_data, sizeInBytes, src, srcSize);
    assert(err == 0);
  }

  // Resize the vector to set its maximum capacity to |size| (or greater).
  void Ensure(int size) {
    Expand(_capacity + size);
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
  if (a._data == b._data) return true; // A and B have the same data pointers (extremely unlikely if they are not also the same)
  for (int i=0; i<a._size; i++) { // Finally, compare every element in A and B.
    if (a[i] != b[i]) return false;
  }
  return true;
}

template <typename T>
class LinkedList {
public:
  // Default constructor for declaration purposes only
  LinkedList() {}
  LinkedList(T* head) : _head(head), _tail(head), _size(1) {}

  T* Head() { return _head; }
  T* Tail() { return _tail; }
  int Size() { return _size; }

  void AddToTail(T* newTail) {
    assert(_tail);
    assert(newTail);
    _tail->next = newTail;
    _tail = newTail;
    _size++;
  }

  void AdvanceHead() {
    assert(_head);
    _head = _head->next;
    _size--;
    assert(_size >= 0);
  }

private:
  T* _head = nullptr;
  T* _tail = nullptr;
  int _size = 0;
};

template <typename T>
class LinkedLoop {
public:
  // Default constructor for declaration purposes only
  LinkedLoop() {}
  LinkedLoop(T* node) { AddToHead(node); }

  T* Previous() { return _previous; }
  T* Current() { return _current; }
  int Size() { return _size; }

  void AddToHead(T* node) {
    assert(node);
    if (_current == nullptr) {
      node->next = node;
      _current = node;
      _previous = node;
      _size = 1;
    } else {
      _previous->next = node;
      node->next = _current;
      _current = node;
      _size++;
    }
    assert(_current->next);
  }

  void Advance() {
    _previous = _current;
    _current = _current->next;
  }

  void Pop() {
    T* next = _current->next;
    _previous->next = next;
    _current = next;
    _size--;
  }

  // Functions for range-based iteration
  struct iterator {
    iterator(T* node) : _node(node) {}

    T* operator*() {
      return _node;
    }

    void operator++() {
      _first = false;
      _node = _node->next;
    }

    bool operator!=(const iterator& other) {
      return _first || _node != other._node;
    }

  private:
    T* _node;
    bool _first = true; // Required so that we don't immediately exit the loop (because begin() == end())
  };
  iterator begin() {
    return iterator(_current);
  }
  iterator end() {
    return iterator(_current);
  }

private:
  T* _previous = nullptr;
  T* _current = nullptr;
  int _size = 0;
};

// Adapated from Matt Kulukundis's (Google) CppCon 2017 talk
using T = u32;
class FlatHashSet {
  enum Ctrl : s8 {
    kEmpty = -128,    // 0b10000000
    kDeleted = -2,    // 0b11111110
    kSentinel = -1,   // 0b11111111
    // kFull =           0b0xxxxxxx
  };

public:
  // Construct an empty hashtable. This allocates space in memory (or on the stack) for the hashtable *container*,
  // but does not allocate any memory for the hashtable *contents*.
  FlatHashSet() { }
  // Construct a hashtable of |capacity|. This allocates space in memory (or on the stack) for the hashtable *container*,
  // AND allocates memory for the hashtable *contents*.
  FlatHashSet(int capacity) {
    _size = 0;
    _capacity = capacity;
    _ctrl = new u8[capacity];
    _slots = new T[capacity];
  }
  ~FlatHashSet() {
    if (_ctrl != nullptr) delete[] _ctrl;
    if (_slots != nullptr) delete[] _slots;
  }

  // Copying should be done with .Copy(), to discourage accidental copying.
  FlatHashSet(const FlatHashSet& other) = delete; /* Copy constructor */
  FlatHashSet& operator=(const FlatHashSet& other) = delete; /* Copy assignment */

  bool TryAdd(T value) {
    u32 hash = triple32_hash(value);
    size_t pos = H1(hash) % _size;
    while (true) {
      if (H2(hash) == _ctrl[pos] && value == _slots[pos]) return false; // Already exists
      if (_ctrl[pos] == kEmpty) {
        _ctrl[pos] = H2(hash);
        _size++;
        if (_size * 8 > _capacity * 7) Resize();
        return true; // Just added
      }
      pos = (pos + 1) % _size;
    }
  }

private:
  // From https://nullprogram.com/blog/2018/07/31/
  u32 triple32_hash(u32 x) {
      x ^= x >> 17;
      x *= 0xed5ad4bbU;
      x ^= x >> 11;
      x *= 0xac4c1b51U;
      x ^= x >> 15;
      x *= 0x31848babU;
      x ^= x >> 14;
      return x;
  }

  void Erase(size_t pos) {
    _size--;
    _ctrl[pos] = kDeleted;
    _slots[pos].~T();
  }

  void Resize() {
    // Abseil uses a "power of 2 minus 1" for capacities. Probably because they have a good chance of being prime or pseudo-prime.
    FlatHashSet newTable(_capacity * 2 + 1);
    for (int i=0; i<_capacity; i++) {
      if (_ctrl[i] & kEmpty) continue; // High bit is set, so the associated slot had no real data.
      T value = _slots[i];
      newTable.TryAdd(value);
    }

    _capacity = newTable._capacity;
    _ctrl = newTable._ctrl;
    _slots = newTable._slots;
    newTable._ctrl = nullptr;
    newTable._slots = nullptr;
  }

  size_t H1(size_t hash) { return hash >> 7; }
  u8 H2(size_t hash) { return (u8)(hash & 0x7F); }

  int _size = 0;
  int _capacity = 0;
  u8* _ctrl = nullptr;
  T* _slots = nullptr;
};

