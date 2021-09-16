#pragma once
#include <iostream>
#include <unordered_map>

#define LINE_NONE 0
#define LINE_BLACK 1
#define LINE_BLUE 2
#define LINE_YELLOW 3
#define DOT_NONE 0
#define DOT_BLACK 1
#define DOT_BLUE 2
#define DOT_YELLOW 3
#define DOT_INVISIBLE 4
#define GAP_NONE 0
#define GAP_BREAK 1
#define GAP_FULL 2

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
  Level _level;

public:
  template <typename... Types> void error(Types... args)   { _log(Level::Error, args...); }
  template <typename... Types> void warning(Types... args) { _log(Level::Warning, args...); }
  template <typename... Types> void info(Types... args)    { _log(Level::Info, args...); }
  template <typename... Types> void log(Types... args)     { _log(Level::Log, args...); }
  template <typename... Types> void debug(Types... args)   { _log(Level::Debug, args...); }
  template <typename... Types> void spam(Types... args)    { _log(Level::Spam, args...); }

  void SetLogLevel(const std::string& level) {
    if (level == "none")  _level = Level::None;
    if (level == "error") _level = Level::Error;
    if (level == "warn")  _level = Level::Warning;
    if (level == "info")  _level = Level::Info;
    if (level == "log")   _level = Level::Log;
    if (level == "debug") _level = Level::Debug;
    if (level == "spam")  _level = Level::Spam;
  }

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

// Functions I wish std::vector had, but it doesn't.
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
      os << ', ';
    }
    first = false;
    os << it;
  }
  os << ']';
  return os;
}

// Functions I wish std::map had

template <typename K, typename V>
V GetValueOrDefault(const std::unordered_map<K, V>& map, K key, V defaultValue) {
  auto search = map.find(key);
  if (search != map.end()) return search->second;
  return defaultValue;
}