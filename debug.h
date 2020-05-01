#ifndef ST_DEBUG_H_
#define ST_DEBUG_H_

#include <iostream>

#ifdef ST_DEBUG
#  define deb(x) std::cout << #x << ": " << (x) << std::endl
#  define debout(x) std::cout << x << std::endl
#  define debline() std::cout << std::endl
#else
#  define deb(x) do { } while(0);
#  define debout(x) do { } while(0);
#  define debline() do { } while(0);
#endif  // ST_DEBUG

#endif  // ST_DEBUG_H_
