#pragma once
#include <utility> // For pair, sadly

using u8 = unsigned char;

enum class Dot    : u8;
enum class Line   : u8;
enum class Gap    : u8;
enum class Sym    : u8;
enum class End    : u8;
enum class Path   : u8;
enum class Type   : u8;
enum class Masked : u8;
struct Cell;
class Puzzle;
class Random;
struct RegionData;
template <typename T> class Vector;

using Region = Vector<Cell*>;

#define DELETE_RO3(clazz) \
  clazz##(const clazz & other) = delete; /* Copy constructor */ \
  clazz & operator=(const clazz & other) = delete; /* Copy assignment */

#define DELETE_RO5(clazz) \
  clazz##(clazz && other) noexcept = delete; /* Move constructor */ \
  clazz & operator=(clazz && other) noexcept = delete; /* Move assignment */

