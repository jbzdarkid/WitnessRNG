#pragma once
#include <iostream>

enum class Line : u8 {
  None =   0,
  Black =  1,
  Blue =   2,
  Yellow = 3,
};

enum class Dot : u8 {
  None =   0,
  Black =  1,
  Blue =   2,
  Yellow = 3,
  Invis =  4,
};

#define GAP_NONE    0
#define GAP_BREAK   1
#define GAP_FULL    2

#define SYM_NONE    0
#define SYM_X       1
#define SYM_Y       2
#define SYM_XY      3

enum class End : u8 {
  None   = 0,
  Left   = 1,
  Right  = 2,
  Top    = 3,
  Bottom = 4,
};

#define PATH_NONE   0
#define PATH_LEFT   1
#define PATH_RIGHT  2
#define PATH_TOP    3
#define PATH_BOTTOM 4

enum class Type : u8 {
  Null =     0,
  Line =     1,
  Square =   2,
  Star =     3,
  Nega =     4,
  Triangle = 5,
  Poly =     6,
  Ylop =     7,
  Nonce =    8,
};

// The masked grid contains 5 colors:
enum class Masked : u8 {
  // null: Out of bounds or already processed
  OutOfBounds = 0xFF,
  Processed = 0xFF,
  // 0: In bounds, awaiting processing, but should not be part of the final region.
  Uncounted = 0,
  // 1: In bounds, awaiting processing
  Counted = 1,
  // 2: Gap-2. After _floodFillOutside, this means "treat normally" (it will be null if oob)
  Gap2 = 2,
  // 3: Dot (of any kind), otherwise identical to 1. Should not be flood-filled through (why the f do we need this)
  Dot = 3,
};


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
  int _depth = 0;

public:
  template <typename... Types> void error(Types... args)   { _logGroup(Error, args...); }
  template <typename... Types> void warning(Types... args) { _logGroup(Warning, args...); }
  template <typename... Types> void info(Types... args)    { _logGroup(Info, args...); }
  template <typename... Types> void log(Types... args)     { _logGroup(Log, args...); }
  template <typename... Types> void debug(Types... args)   { _logGroup(Debug, args...); }
  template <typename... Types> void spam(Types... args)    { _logGroup(Spam, args...); }

  void group() { _depth += 2; }
  void groupEnd() { _depth -= 2; }

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
  template <typename... Types>
  void _logGroup(Level level, Types... args) {
    if (level > _level) return;
    std::cout << std::string(_depth, ' ');
    _log(level, args...);
  }

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
/*
#include <unordered_map>
template <typename K, typename V>
V GetValueOrDefault(const std::unordered_map<K, V>& map, const K& key, V defaultValue) {
  auto search = map.find(key);
  if (search != map.end()) return search->second;
  return defaultValue;
}
*/

// Functions I wish std::set had
#include <unordered_set>
template <typename T>
bool Contains(const std::unordered_set<T>& set, const T& value) {
  return set.find(value) != set.end();
}
