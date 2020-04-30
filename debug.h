#ifndef ST_DEBUG_H_
#define ST_DEBUG_H_

#include <iostream>

#define deb(x) std::cout << #x << ": " << (x) << std::endl
#define debout(x) std::cout << x << std::endl
#define debline() std::cout << std::endl

#endif  // ST_DEBUG_H_
