#ifndef ST_UTIL_H_
#define ST_UTIL_H_

#include <type_traits>

template<typename T>
void assert_between_close_open(T left_closed, T x, T right_open) {
  assert(left_closed <= x);
  assert(x < right_open);
}

template<typename T>
std::make_unsigned_t<T> unsigned_or_die(T a) {
  std::make_unsigned_t<T> ret = static_cast<std::make_unsigned_t<T>>(a);
  assert(a == static_cast<T>(ret));
  return ret;
}

#endif  // ST_UTIL_H_
