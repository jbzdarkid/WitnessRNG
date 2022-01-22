#pragma once
#include <utility> // For move
#include <initializer_list> // For initializer constructor
#include <cstring> // For memset, memcpy
#include <new> // for std::bad_alloc

#ifndef assert
#ifdef _DEBUG
#define assert(cond) \
{ \
  if (!(cond)) { \
    __debugbreak(); \
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
    if (_size == 0) return nullptr;
    return &_data[0];
  }
  const T* begin() const {
    if (_size == 0) return nullptr;
    return &_data[0];
  }
  T* end() {
    if (_size == 0) return nullptr;
    return &_data[_size];
  }
  const T* end() const {
    if (_size == 0) return nullptr;
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

  // Fill the entire vector's contents with |value|.
  void Fill(const T& value) {
    if constexpr (sizeof(T) == 1) {
      memset(_data, value, sizeof(T) * _capacity);
    } else { // Memset operates only on bytes, so we have to fall back if we want to set a larger type.
      for (int i=0; i<_capacity; i++) _data[i] = value;
    }
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
  int _capacity = 0; // This class member should never be visible, it is an implementation detail.
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
  LinkedList(T* head) : _head(head), _tail(head), _size(1) {
    assert(head);
  }

  T* Head() { return _head; }
  T* Tail() { return _tail; }
  int Size() { return _size; }

  void AddToTail(T* newTail) {
    assert(newTail);
    _tail->next = newTail;
    _tail = newTail;
    _size++;
  }

  void AdvanceHead() {
    assert(_size > 0);
    _head = _head->next;
    _size--;
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
  LinkedLoop(T* node) { AddCurrent(node); }

  T* Previous() { return _previous; }
  T* Current() { return _current; }
  int Size() { return _size; }

  void AddCurrent(T* node) {
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


template <typename T>
class LinearAllocator {
public:
  LinearAllocator() {
    _firstBucket = _lastBucket = Bucket::New(16);
  }

  ~LinearAllocator() {
    Bucket* b = _firstBucket;
    while (b) {
      Bucket* next = b->next;
      free(b);
      b = next;
    }
  }

  // Copying does not make sense for this type. Make a new one if you want another.
  LinearAllocator(const LinearAllocator& other) = delete; /* Copy constructor */
  LinearAllocator& operator=(const LinearAllocator& other) = delete; /* Copy assignment */

  T* allocate(size_t n) {
    assert(n == 1); // for now
    if (_lastBucket->size + n > _lastBucket->capacity) NewBucket(_lastBucket->capacity * 2);
    T* data = &_lastBucket->data;
    T* addr = &data[_lastBucket->size];
    _lastBucket->size += (u32)n;
    return addr;
  }

  void deallocate(T* const addr, size_t n) {
    // free!
  }

private:
  struct Bucket {
    static Bucket* New(u32 capacity) {
      Bucket* b = (Bucket*)malloc(sizeof(Bucket) + sizeof(T) * (capacity - 1));
      if (b == nullptr) throw std::bad_alloc();
      b->size = 0;
      b->capacity = capacity;
      b->next = nullptr;
      return b;
    }
    u32 size = 0;
    u32 capacity = 0;
    Bucket* next;
    T data; // This is actually an array of length |capacity|.
  };

  void NewBucket(u32 capacity) {
    Bucket* b = Bucket::New(capacity);
    _lastBucket->next = b;
    _lastBucket = b;
  }

  Bucket* _firstBucket;
  Bucket* _lastBucket;
};

// Adapted from Matt Kulukundis's (Google) CppCon 2017 talk
template <typename T>
class FlatHashSet {
  static_assert(not std::is_pointer<T>());

  enum Ctrl : u8 {
    kEmpty =    0b10000000,
    kDeleted =  0b11111110,
    kSentinel = 0b11111111,
    // kFull =  0b0xxxxxxx
  };

public:
  // Construct a hashtable of |capacity| (default 7).
  FlatHashSet(int capacity = 7) {
    _size = 0;
    _capacity = capacity;
    _ctrl = new u8[capacity];
    memset(_ctrl, kEmpty, capacity);
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
    size_t hash = std::hash<T>()(value);
    size_t pos = 0;
    bool found = Find(value, hash, pos);
    if (found) return false; // Already exists
    
    _ctrl[pos] = H2(hash);
    _slots[pos] = value; // NB: Copies the value
    _size++;
    if (_size * 8 > _capacity * 7) Resize();
    return true; // Just added
  }

private:
  /*
  void Remove(const T& value) {
    size_t hash = std::hash<T>()(value);
    size_t pos = 0;
    bool found = Find(value, hash, pos);
    if (!found) return;

    _size--;
    _ctrl[pos] = kDeleted;
    _slots[pos].~T();
  }
  */

  bool Find(const T& value, size_t hash, size_t& pos) {
    pos = H1(hash) % _capacity;
    while (true) {
      if (H2(hash) == _ctrl[pos] && value == _slots[pos]) return true; // Exists here!
      if (_ctrl[pos] == kEmpty) return false; // Insert here!
      pos = (pos + 1) % _capacity;
    }
  }

  void Resize() {
    // Abseil uses a "power of 2 minus 1" for capacities. Probably because they have a good chance of being prime or pseudo-prime.
    FlatHashSet newTable((_capacity + 1) * 2 - 1);
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

// Adapted from Matt Kulukundis's (Google) CppCon 2017 talk
template <typename T>
class NodeHashSet {
  enum Ctrl : u8 {
    kEmpty =    0b10000000,
    kDeleted =  0b11111110,
    kSentinel = 0b11111111,
    // kFull =  0b0xxxxxxx
  };

public:
  // Construct a hashtable of |capacity| (default 7).
  NodeHashSet(int capacity = 7) {
    _size = 0;
    _capacity = capacity;
    _ctrl = new u8[capacity];
    memset(_ctrl, kEmpty, capacity);
    _slots = new T*[capacity];
  }
  ~NodeHashSet() {
    if (_slots != nullptr && _ctrl != nullptr) {
      for (size_t pos = 0; pos < _capacity; pos++) {
        if ((_ctrl[pos] & kEmpty) == 0) {
          delete _slots[pos];
        }
      }
    }
    if (_slots != nullptr) delete[] _slots;
    if (_ctrl != nullptr) delete[] _ctrl;
  }

  // Copying should be done with .Copy(), to discourage accidental copying.
  NodeHashSet(const NodeHashSet& other) = delete; /* Copy constructor */
  NodeHashSet& operator=(const NodeHashSet& other) = delete; /* Copy assignment */

  // If |value| does not exist in the hashset, insert a copy of it.
  // This makes a copy of |value| on the heap.
  // The value (freshly inserted or not) is returned via |heapValue| (if not null).
  bool CopyAdd(const T& value, T** heapValue = nullptr) {
    size_t hash, pos;
    if (Find(&value, hash, pos)) { // Already exists
      if (heapValue) *heapValue = _slots[pos];
      return false;
    }
    
    T* newValue;
    try {
      newValue = _allocator.allocate(1);
    } catch (std::bad_alloc&) {
      return false;
    }
    newValue = new (newValue) T(value); // Allocation via copy constructor
    if (heapValue) *heapValue = newValue;
    Insert(pos, hash, newValue);
    return true; // Just added
  }

  // If |value| does not exist in the hashset, insert it.
  // This does not copy |value|, but it does assume that the pointer is heap-allocated.
  bool TryAdd(T* value) {
    size_t hash, pos;
    if (Find(value, hash, pos)) return false; // Already exists

    Insert(pos, hash, value); // Does not copy the value
    return true; // Just added
  }

  size_t Size() const {
    return _size;
  }

private:
  /*
  void Remove(const T* value) {
    size_t hash = std::hash<T>()(*value);
    size_t pos = 0;
    bool found = Find(value, hash, pos);
    if (!found) return;

    _size--;
    _ctrl[pos] = kDeleted;
    _slots[pos] = nullptr; // Technically unnecessary.
  }
  */

  bool Find(const T* value, size_t& hash, size_t& pos) {
    hash = std::hash<T>()(*value);
    pos = H1(hash) % _capacity;
    while (true) {
      if (H2(hash) == _ctrl[pos] && *value == *_slots[pos]) return true; // Exists here!
      if (_ctrl[pos] == kEmpty) return false; // Insert here!
      ++pos;
      if (pos > _capacity) pos -= _capacity; // Extremely obvious branch prediction is faster than modulo here.
    }
  }

  void Insert(size_t pos, size_t hash, T* value) {
    _ctrl[pos] = H2(hash);
    _slots[pos] = value; 
    _size++;
    if (_size * 8 > _capacity * 7) Resize();
  }

  void Resize() {
    // Abseil uses a "power of 2 minus 1" for capacities. Probably because they have a good chance of being prime or pseudo-prime.
    NodeHashSet newTable((_capacity + 1) * 2 - 1);
    for (int i=0; i<_capacity; i++) {
      if (_ctrl[i] & kEmpty) continue; // High bit is set, so the associated slot had no real data.
      T* value = _slots[i];
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
  LinearAllocator<T> _allocator;
  u8* _ctrl = nullptr;
  T** _slots = nullptr;
};
