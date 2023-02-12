#pragma once
#include <utility> // For move
#include <initializer_list> // For initializer constructor
#include <cstring> // For memset, memcpy
#include <new> // for std::bad_alloc

using u8 = unsigned char;
using s8 = signed char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using s64 = long long;

#ifndef assert
#ifdef _DEBUG
#define assert(cond) \
do { \
  if (!(cond)) { \
    __debugbreak(); \
  } \
} while (0)
#else
#define assert(cond) do {} while (0)
#endif
#endif // #ifndef assert

template <typename T>
class LinearAllocator {
public:
  LinearAllocator(u32 initialCapacity = 16) {
    _firstArray = _lastArray = Array::New(initialCapacity * sizeof(T));
  }

  ~LinearAllocator() {
    Array::Delete(_firstArray);
  }

  // Copying does not make sense for this type. Make a new one if you want another.
  LinearAllocator(const LinearAllocator& other) = delete; // Copy constructor
  LinearAllocator& operator=(const LinearAllocator& other) = delete; // Copy assignment

  T* allocate(size_t n) {
    if (_lastArray->size + n > _lastArray->capacity) {
      assert(n == (u32)n);
      if (n > _lastArray->capacity) NewArray((u32)n); // Asking for a large amount of data
      else NewArray(_lastArray->capacity * 2); // Asking for a small amount of data
    }
    T* data = &_lastArray->data;
    T* addr = &data[_lastArray->size];
    _lastArray->size += (u32)n;
    return addr;
  }

  void deallocate(T* const addr, size_t n) {
    // free!
  }

private:
  struct Array {
    static Array* New(u32 capacity) {
      Array* b = (Array*)malloc(sizeof(Array) + sizeof(T) * (capacity - 1));
      if (b == nullptr) throw std::bad_alloc();
      b->size = 0;
      b->capacity = capacity;
      b->next = nullptr;
      return b;
    }

    static void Delete(Array* a) {
      if (!a) return;
      Array* next = a->next;
      a->next = nullptr;
      free(a);
      Delete(next);
    }

    u32 size = 0;
    u32 capacity = 0;
    Array* next;
    T data; // This is actually an array of length |capacity|.
  };

  void NewArray(u32 capacity) {
    Array* a = Array::New(capacity);
    _lastArray->next = a;
    _lastArray = a;
  }

  Array* _firstArray;
  Array* _lastArray;
};

template <typename T>
class NArray {
public:
  NArray() {}
  NArray(u8 maxA, u8 maxB=1, u8 maxC=1, u8 maxD=1) {
    assert(maxA > 0);
    assert(maxB > 0);
    assert(maxC > 0);
    assert(maxD > 0);
    _maxA = maxA;
    _maxB = maxB;
    _maxC = maxC;
    _maxD = maxD;
    _data = (T*)malloc(sizeof(T) * maxA * maxB * maxC * maxD);
  }
  ~NArray() {
    if (_data != nullptr) free(_data);
  }
  // Copy constructors do not work if your class has raw pointers.
  NArray(const NArray& other) = delete; // Copy constructor
  NArray& operator=(const NArray& other) = delete; // Copy assignment

  // So set up move operators instead
  NArray(NArray&& other) noexcept { // Move constructor
    other = std::move(this);
  }
  NArray& operator=(NArray&& other) noexcept { // Move assignment
    _maxA = other._maxA;
    _maxB = other._maxB;
    _maxC = other._maxC;
    _maxD = other._maxD;
    _data = other._data;
    other._data = nullptr;
    return *this;
   }

  const T& Get(u8 a, u8 b = 0, u8 c = 0, u8 d = 0) const {
    assert(a < _maxA);
    assert(b < _maxB);
    assert(c < _maxC);
    assert(d < _maxD);
    int index = a;
    if (_maxB > 1) index = index * _maxB + b;
    if (_maxC > 1) index = index * _maxC + c;
    if (_maxD > 1) index = index * _maxD + d;
    assert(index == ((a * _maxB + b) * _maxC + c) * _maxD + d);
    assert(index < _maxA * _maxB * _maxC * _maxD);
    return _data[index];
  }

  const T* GetRow(u8 a, u8 b = 0, u8 c = 0, u8 d = 0) const {
    if (c != 0) assert(d == 0); // Accessing a 3D row as a 4D array
    else if (b != 0) assert(c == 0 && _maxD == 1); // Accessing a 2D row in a 3D array
    else if (a != 0) assert(b == 0 && _maxC == 1); // Accessing a 1D row in a 2D array
    return &Get(a, b, c, d);
  }

  const T& operator()(u8 a, u8 b=0, u8 c=0, u8 d=0) const {
    return Get(a, b, c, d);
  }

  T& Get(u8 a, u8 b = 0, u8 c = 0, u8 d = 0) {
    const auto& const_this = const_cast<const NArray<T>&>(*this); // Create a const copy of ourselves, so the compiler knows to call the other overload
    const T& const_ret = const_this.Get(a, b, c, d); // Call the const method, which returns const data
    return const_cast<T&>(const_ret); // But we know we weren't actually const, so return a mutable copy.
  }

  T* GetRow(u8 a, u8 b = 0, u8 c = 0, u8 d = 0) {
    const auto& const_this = const_cast<const NArray<T>&>(*this); // Create a const copy of ourselves, so the compiler knows to call the other overload
    const T* const_ret = const_this.GetRow(a, b, c, d); // Call the const method, which returns const data
    return const_cast<T*>(const_ret); // But we know we weren't actually const, so return a mutable copy.
  }

  T& operator()(u8 a, u8 b=0, u8 c=0, u8 d=0) {
    return Get(a, b, c, d);
  }

  void Fill(const T& value) {
    assert(_data != nullptr);
    size_t size = _maxA * _maxB * _maxC * _maxD;
    if constexpr (sizeof(T) == 1) {
      memset(_data, (s8)value, size);
    } else { // Memset operates only on bytes, so we have to fall back if we want to set a larger type.
      for (int i=0; i<size; i++) _data[i] = value;
    }
  }

private:
  int _maxA = 0;
  int _maxB = 0;
  int _maxC = 0;
  int _maxD = 0;
  T* _data = nullptr;
};

template <typename T>
class Vector {
public:
  // Construct an empty vector. This allocates space in memory (or on the stack) for the vector *container*,
  // but does not allocate any memory for the vector *contents*.
  Vector() {}
  // Construct a vector of |capacity|. This allocates space in memory (or on the stack) for the vector *container*,
  // AND allocates memory for the vector *contents*.
  Vector(int capacity) {
    _size = 0;
    _capacity = capacity;
    _data = new T[capacity];
  }
  // Construct a vector of maximum |capacity|, using allocator |alloc|.
  // This allocates space in memory (or on the stack) for the vector *container*,
  // BUT uses the allocator to allocate memory for the vector *contents*.
  Vector(int capacity, LinearAllocator<T>& alloc) {
    _size = 0;
    _capacity = capacity;
    void* addr = alloc.allocate(capacity * sizeof(T));
    _data = new (addr) T[capacity];
  }
  // Construct a vector from an initializer list. This allocates exactly enough space for the initializer contents.
  Vector(std::initializer_list<T> init) : Vector((int)init.size()) {
    for (const T& it : init) UnsafePush(it);
  }
  ~Vector() {
    if (_data != nullptr) delete[] _data;
  }

  // Copying should be done with .Copy(), to discourage accidental copying.
  Vector(const Vector& other) = delete; // Copy constructor
  Vector& operator=(const Vector& other) = delete; // Copy assignment

  // Moving is OK, we need it for when we have a Vector<Vector<>>
  Vector(Vector&& other) noexcept { // Move constructor
    _size = other._size;
    _capacity = other._capacity;
    // TODO: Are we leaking _data here?
    _data = other._data;
    other._data = nullptr;
  }
  Vector& operator=(Vector&& other) noexcept { // Move assignment
    _size = other._size;
    _capacity = other._capacity;
    // TODO: Are we leaking _data here?
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

  // *** Cheap functions I use *** //

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
  [[nodiscard]] T PopValue() {
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
  // WARNING: If you resize down, any data beyond the |size| will be inaccessible! If it is heap-allocated,
  // be sure to delete/free it before downsizing.
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

  // *** Moderately expensive functions *** //

  bool Contains(const T& value) const {
    return IndexOf(value) != -1;
  }

  int IndexOf(const T& value) const {
    for (int i=0; i<_size; i++) {
      if (_data[i] == value) return i;
    }
    return -1;
  }


  // Special case of Fill which also works on Vectors of nontrivial types.
  // Sets all bytes in the entire vector to |byte|.
  void Fill(u8 byte) {
    assert(_size <= _capacity);
    memset(_data, byte, _size * sizeof(T));
  }

  // Fill the vector up to its size with |value|. Does not change the vector's capacity.
  void Fill(const T& value) {
    assert(_size <= _capacity);
    if constexpr (sizeof(T) == 1) {
      memset(_data, value, _size);
    } else { // Memset operates only on bytes, so we have to fall back if we want to set a larger type.
      for (int i=0; i<_size; i++) _data[i] = value;
    }
  }

  // *** Expensive functions *** //

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

  // Sort and copy the contents of the vector into |dest|, a raw array containing |destSize| bytes
  using CompareFunc = s8 (*)(const T&, const T&);
  void SortedCopyIntoArray(T* dest, size_t destSize, CompareFunc cmp) const {
    assert(destSize == _size * sizeof(T));

    // Insertion sort, because it doesn't require any additional memory.
    dest[0] = _data[0];
    for (int i = 1; i<_size; i++) {
      T& a = _data[i];
      int j = i;
      do {
        T& b = dest[j-1];
        if (cmp(a, b) >= 0) break; // a >= b
        dest[j] = b;
        j--;
      } while (j > 0);
      dest[j] = a;
    }
  }

  // Set the contents of this vector from |dest|, a raw array containing |destSize| bytes
  void CopyFromArray(const T* src, size_t srcSize) {
    size_t sizeInBytes = sizeof(T) * _size;
    assert(srcSize == sizeInBytes);
    int err = memcpy_s(_data, sizeInBytes, src, srcSize);
    assert(err == 0);
  }

  // Resize the vector such that its capacity is |required| or greater.
  // If |required| is less than the current capacity, no resize is performed.
  void Ensure(int required) {
    Expand(required - _capacity);
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
bool operator!=(const Vector<T>& a, const Vector<T>& b) { return !(a == b); }

template <typename T>
class LinkedList {
public:
  LinkedList() {}

  T* Head() { return _head; }
  T* Tail() { return _tail; }
  int Size() { return _size; }

  void AddToTail(T* newTail) {
    assert(newTail);
    if (_tail == nullptr) { // UNLIKELY()
      assert(_head == nullptr);
      _head = newTail;
      _tail = newTail;
    } else {
      _tail->next = newTail;
      _tail = newTail;
    }
    _size++;
  }

  void AdvanceHead() {
    assert(_size > 0);
    _head = _head->next;
    _size--;
    if (_head == nullptr) {
      assert(_size == 0);
      _tail = nullptr; // If we empty the entire LinkedList, we need to clear _tail too.
    }
  }

  // Functions for range-based iteration
  struct iterator {
    iterator(T* node) : _node(node) {}

    T* operator*() {
      return _node;
    }

    void operator++() {
      _node = _node->next;
    }

    bool operator!=(const iterator& other) {
      return _node != other._node;
    }

  private:
    T* _node;
  };
  iterator begin() {
    return iterator(_head);
  }
  iterator end() {
    return iterator(_tail);
  }

private:
  T* _head = nullptr;
  T* _tail = nullptr;
  int _size = 0;
};

template <typename T>
class LinkedLoop {
public:
  LinkedLoop() {}

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

  /* I don't think this works at _size == 1, sadly
  T* Pop() {
    T* toPop = _current;
    _previous->next = toPop->next;
    _current = toPop->next;
    _size--;
    toPop->next = nullptr;
    return toPop;
  }
  */

  // Functions for range-based iteration
  struct iterator {
    iterator(T* node) : _node(node) {
      if (node) _first = true; // Required so that we don't immediately exit the loop (because begin() == end())
    }

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
    bool _first = false;
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
  NodeHashSet(size_t capacity = 7) {
    _size = 0;
    _capacity = capacity;
    _ctrl = new u8[capacity];
    memset(_ctrl, kEmpty, capacity);
    _slots = new T*[capacity];
  }
  ~NodeHashSet() {
    if (_slots != nullptr && _ctrl != nullptr) {
      for (size_t pos = 0; pos < _capacity; pos++) {
        if ((_ctrl[pos] & kEmpty) == 0) { // High bit is not set so the slot is not empty
          assert(_slots[pos] != nullptr);
          _slots[pos]->~T();
          _allocator.deallocate(_slots[pos], sizeof(T));
        }
      }
    }
    if (_slots != nullptr) delete[] _slots;
    if (_ctrl != nullptr) delete[] _ctrl;
  }

  // Copying should be done with .Copy(), to discourage accidental copying.
  NodeHashSet(const NodeHashSet& other) = delete; // Copy constructor
  NodeHashSet& operator=(const NodeHashSet& other) = delete; // Copy assignment

  // If |value| does not exist in the hashset, insert a copy of it.
  // This makes a copy of |value| on the heap, using a private LinearAllocator.
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
    newValue = new (newValue) T(value); // Initialization via copy constructor
    if (heapValue) *heapValue = newValue;
    Insert(pos, hash, newValue);
    return true; // Just added
  }

#if 0
  // If |value| does not exist in the hashset, insert it.
  // This does not copy |value|, but it does assume that the pointer is heap-allocated.
  bool TryAdd(T* value) {
    size_t hash, pos;
    if (Find(value, hash, pos)) return false; // Already exists

    Insert(pos, hash, value); // Does not copy the value
    return true; // Just added
  }
#endif

  size_t Size() const {
    return _size;
  }

private:
  bool Remove(const T* value) {
    size_t hash, pos;
    if (!Find(value, hash, pos)) return false; // Doesn't exist

    // I have not fully thought out the implications of setting something to kDeleted.
    // Until then, do not use this function.
    assert(false);
    _ctrl[pos] = kDeleted;
#if _DEBUG
    _slots[pos] = nullptr;
#endif
    _size--;
    return true;
  }

#if 0
#define HIT _hits++
#define MISS _misses++
#define H1_COLLIDE _h1collisions++
#define H2_COLLIDE _h2collisions++
  u64 _hits = 0;
  u64 _misses = 0;
  u64 _h1collisions = 0;
  u64 _h2collisions = 0;
#else
#define HIT
#define MISS
#define H1_COLLIDE
#define H2_COLLIDE
#endif

  bool Find(const T* value, size_t& hash, size_t& pos) {
    hash = std::hash<T>()(*value);
    pos = H1(hash) % _capacity;
    while (true) {
      if (H2(hash) == _ctrl[pos]) {
        if (*value == *_slots[pos]) { HIT; return true; }
        H2_COLLIDE;
      }
      if (_ctrl[pos] == kEmpty) { MISS; return false; }
      H1_COLLIDE;
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
    for (size_t i=0; i<_capacity; i++) {
      if (_ctrl[i] & kEmpty) continue; // High bit is set, so the associated slot had no real data.
      T* value = _slots[i];
      newTable.ResizeAdd(value);
#if _DEBUG
      _slots[i] = nullptr;
      _ctrl[i] = kDeleted;
#endif
    }

    _capacity = newTable._capacity;
    _ctrl = newTable._ctrl;
    _slots = newTable._slots;
    newTable._ctrl = nullptr;
    newTable._slots = nullptr;
  }

  // During a resize, we can avoid most of the safety checks. We know that the item doesn't yet exist,
  // so we don't need to worry about hash collisions: If the slot is filled then it cannot be this value.
  void ResizeAdd(T* value) {
    size_t hash = std::hash<T>()(*value);
    size_t pos = H1(hash) % _capacity;
    while (true) {
      if (_ctrl[pos] == kEmpty) break;
      ++pos;
      if (pos > _capacity) pos -= _capacity; // Extremely obvious branch prediction is faster than modulo here.
    }
    _ctrl[pos] = H2(hash);
    _slots[pos] = value;
  }

  size_t H1(size_t hash) { return hash >> 7; }
  u8 H2(size_t hash) { return (u8)(hash & 0x7F); }

  size_t _size = 0;
  size_t _capacity = 0;
  LinearAllocator<T> _allocator;
  u8* _ctrl = nullptr;
  T** _slots = nullptr;
};
