#include <cstdlib>
#include <cassert>
#include <iostream>

#include <fstream>

#include "ukkonen.h"

#include "debug.h"

void uk(std::string);

void uk(std::string str) {

  // srand(1231231);
  // std::cin >> str;
  // str = "abcabx";
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

std::vector<uint8_t> read_file(std::string file_name);

std::vector<uint8_t> read_file(std::string file_name) {
  std::ifstream is(file_name, std::fstream::binary);
  is.seekg (0, is.end);
  auto length = is.tellg();
  is.seekg (0, is.beg);

  assert(length > 0);
  char *buffer = new char [static_cast<size_t>(length)];
  // read data as a block:
  is.read (buffer, length);
  assert(is);

  auto ret = std::vector<uint8_t>(buffer, buffer + static_cast<size_t>(length));

  delete[] buffer;

  return ret;
}

int main(int argc, char **argv) {

  if(argc == 1) {
    std::vector<std::string> strs = {
      "something",
      "ississ",
      "ississ$",
      "abcabxabcd$",
      "abcdefabxybcdmnabcdex",
      "abcabxabcd",
    };

    for(auto it: strs) {
      deb(it);
      uk(it);
      debline();
    }
    return 0;
  }

  assert(argc >= 3);
  std::string target_fname = argv[1];
  std::vector<std::string> bases;
  for(int i = 2; i <argc; i++) {
    bases.push_back(argv[i]);
  }

  std::vector<uint16_t> base_content;
  for(auto base_fname: bases) {
    auto content = read_file(base_fname);
    for(auto& it: content) {
      if(it == 0) {
        it = 15;
      }
    }
    base_content.insert(base_content.end(), content.begin(), content.end());
    base_content.push_back(256); // $
  }

  auto target = read_file(target_fname);
  std::vector<uint16_t> target_content(target.begin(), target.end());

  auto st = std::make_shared<Ukkonen<uint16_t>>(base_content);

  auto be = target_content.begin();
  auto fin = target_content.end();

  std::map<int, int> sizes;
  while(be != fin ){
    auto orig = be;
    be = st->max_common_prefix(be, fin);
    sizes[be - orig] += 1;
    if(be == orig) {
      ++be;
    }
  }

  for(auto& it: sizes) {
    std::cout << it.first << " : " << it.second << std::endl;
  }

  std::cout << "done" << std::endl;

  std::vector<long long> parts;

  return 0;
}
