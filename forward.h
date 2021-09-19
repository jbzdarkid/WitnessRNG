#pragma once
#include <utility> // For pair, sadly

using u8 = unsigned char;

enum class Dot : u8;
enum class Line : u8;
struct Cell;
class Puzzle;
class Random;
template <typename T> class Vector;


using Region = Vector<std::pair<u8, u8>>;
using Path = u8*;

#define DELETE_RO3(clazz) \
  clazz##(const clazz & other) = delete; /* Copy constructor */ \
  clazz & operator=(const clazz & other) = delete; /* Copy assignment */

#define DELETE_RO5(clazz) \
  clazz##(clazz && other) noexcept = delete; /* Move constructor */ \
  clazz & operator=(clazz && other) noexcept = delete; /* Move assignment */

