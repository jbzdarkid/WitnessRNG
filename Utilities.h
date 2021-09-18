#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>

using u8 = unsigned char;
enum class Line : u8 {
  None = 0,
  Black = 1,
  Blue = 2,
  Yellow = 3,
};

enum class Dot : u8 {
  None = 0,
  Black = 1,
  Blue = 2,
  Yellow = 3,
  Invis = 4,
};

#define GAP_NONE    0
#define GAP_BREAK   1
#define GAP_FULL    2

#define SYM_NONE    0
#define SYM_X       1
#define SYM_Y       2
#define SYM_XY      3

#define END_NONE    0
#define END_LEFT    1
#define END_RIGHT   2
#define END_TOP     3
#define END_BOTTOM  4

#define PATH_NONE   0
#define PATH_LEFT   1
#define PATH_RIGHT  2
#define PATH_TOP    3
#define PATH_BOTTOM 4

#define CELL_TYPE_NULL     0
#define CELL_TYPE_LINE     1
#define CELL_TYPE_SQUARE   2
#define CELL_TYPE_STAR     3
#define CELL_TYPE_NEGA     4
#define CELL_TYPE_TRIANGLE 5
#define CELL_TYPE_POLY     6
#define CELL_TYPE_YLOP     7
#define CELL_TYPE_NONCE    8

class Console {
  enum Level {
    Error,
    Warning,
    Info,
    Log,
    Debug,
    Spam,
    None,
  };
  Level _level = Info; // Change this value from the Autos tab.

public:
  template <typename... Types> void error(Types... args)   { _log(Error, args...); }
  template <typename... Types> void warning(Types... args) { _log(Warning, args...); }
  template <typename... Types> void info(Types... args)    { _log(Info, args...); }
  template <typename... Types> void log(Types... args)     { _log(Log, args...); }
  template <typename... Types> void debug(Types... args)   { _log(Debug, args...); }
  template <typename... Types> void spam(Types... args)    { _log(Spam, args...); }

  /*
  void SetLogLevel(const std::string& level) {
    if (level == "none")  _level = None;
    if (level == "error") _level = Error;
    if (level == "warn")  _level = Warning;
    if (level == "info")  _level = Info;
    if (level == "log")   _level = Log;
    if (level == "debug") _level = Debug;
    if (level == "spam")  _level = Spam;
  }
  */

private:
  template <typename T, typename... Types>
  void _log(Level level, T value, Types... args) {
    if (level > _level) return;
    std::cout << value << ' ';
    _log(level, args...);
  }

  template <typename T>
  void _log(Level level, T value) {
    if (level > _level) return;
    std::cout << value << std::endl;
  }
};

extern Console console;

#define DELETE_RO3(clazz) \
  clazz##(const clazz & other) = delete; /* Copy constructor */ \
  clazz & operator=(const clazz & other) = delete; /* Copy assignment */

#define DELETE_RO5(clazz) \
  clazz##(clazz && other) noexcept = delete; /* Move constructor */ \
  clazz & operator=(clazz && other) noexcept = delete; /* Move assignment */


// Functions I wish std::vector had

template <typename T>
std::vector<T> Append(std::vector<T>& dest, const std::vector<T>& source) {
  dest.insert(dest.end(), source.begin(), source.end());
  return dest;
}

template <typename T>
T Pop(std::vector<T>& src) {
  T back = src.back();
  src.pop_back();
  return back;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
  os << '[';
  bool first = true;
  for (const T& it : vec) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << it;
  }
  os << ']';
  return os;
}

// Functions I wish std::map had

template <typename K, typename V>
V GetValueOrDefault(const std::unordered_map<K, V>& map, const K& key, V defaultValue) {
  auto search = map.find(key);
  if (search != map.end()) return search->second;
  return defaultValue;
}

// Functions I wish std::set had

template <typename T>
bool Contains(const std::unordered_set<T>& set, const T& value) {
  return set.find(value) != set.end();
}