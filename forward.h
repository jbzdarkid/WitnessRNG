#pragma once
#define _HAS_STD_BYTE 0 // sigh, windows.

#include <vector>
#include <utility>

using u8 = unsigned char;
using s8 = signed char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using s64 = long long;

enum class Dot    : u8;
enum class Line   : u8;
enum class Gap    : u8;
enum class End    : u8;
enum class Type   : u8;
enum class Masked : u8;
struct Cell;
class Puzzle;
class Random;
struct RegionData;
template <typename T> class Vector;

using Region = Vector<Cell*>;
using Path = Vector<u8>;
using Polyomino = std::vector<std::pair<s8, s8>>;

#define DELETE_RO3(clazz) \
  clazz##(const clazz & other) = delete; /* Copy constructor */ \
  clazz & operator=(const clazz & other) = delete; /* Copy assignment */

#define DELETE_RO5(clazz) \
  clazz##(clazz && other) noexcept = delete; /* Move constructor */ \
  clazz & operator=(clazz && other) noexcept = delete; /* Move assignment */

