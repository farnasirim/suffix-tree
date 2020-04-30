#include <cstdlib>
#include <cassert>
#include <iostream>

#include <iostream>

#include "ukkonen.h"

#include "debug.h"

void uk();

void uk() {
  srand(1231231);
  std::string str = "something";
  str = "abcabxabcd$";
  // std::vector<uint16_t> strnums(str.begin(), str.end());
  // for(auto it: strnums) {
  //   std::cout << it << " ";
  // }
  // std::cout << std::endl;
  // auto st = std::make_shared<Ukkonen<uint16_t>>(strnums);

  std::vector<char> strnums(str.begin(), str.end());
  for(auto it: strnums) {
    std::cout << it << " ";
  }
  std::cout << std::endl;
  auto st = std::make_shared<Ukkonen<char>>(strnums);

  std::cout << "done " << std::endl;

  st->dfs();

  // for(int i = 0; i < 10000; i++) {
  //   size_t mx_len = str.size();
  //   size_t current_len = static_cast<unsigned int>(rand()) % mx_len + 1;
  //   std::string needle;
  //   if(static_cast<unsigned int>(rand()) & 1) {
  //     size_t start_at = static_cast<unsigned int>(rand()) % (mx_len - current_len + 1);
  //     needle = str.substr(start_at, current_len);
  //   } else {
  //     for(size_t k = 0; k < current_len; k++) {
  //       needle += static_cast<unsigned int>(rand()) % ('z' - 'a' + 1) + 'a';
  //     }
  //   }
  //   int expected = str.find(needle) != std::string::npos;
  //   int actual = st.find(std::vector<uint8_t>(needle.begin(), needle.end()));

  //   if(expected != actual) {
  //     deb(str);
  //     deb(needle);
  //     deb(expected);
  //     deb(actual);
  //     assert(false);
  //   } else {
  //     deb(needle);
  //   }
  // }
}

int main() {

  uk();

  return 0;
}
