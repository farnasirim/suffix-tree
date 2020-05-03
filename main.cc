#include <cstdlib>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#include "ukkonen.h"

#include "debug.h"

void uk(std::string);

const size_t fixed_overhead = 5;
const size_t complex_overhead = 9;

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

//   std::vector<char> strnums(str.begin(), str.end());
//   for(auto it: strnums) {
//     std::cout << it << " ";
//   }
//   std::cout << std::endl;
//   auto st = std::make_shared<Ukkonen<char>>(strnums);
// 
//   std::cout << "done " << std::endl;
// 
//   // st->dfs();
}

std::vector<uint8_t> read_file(std::string file_name);

std::vector<uint8_t> read_file(std::string file_name) {
  std::ifstream is(file_name, std::fstream::binary);
  is.seekg (0, is.end);
  auto length = is.tellg();
  is.seekg (0, is.beg);

  if(!(length > 0)) {
    std::cout << file_name << " cannot be read" << std::endl;
    assert(false);
  }

  char *buffer = new char [static_cast<size_t>(length)];
  // read data as a block:
  is.read (buffer, length);
  assert(is);

  auto ret = std::vector<uint8_t>(buffer, buffer + static_cast<size_t>(length));

  delete[] buffer;

  return ret;
}

void test_uk();
void test_uk() {
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
}

void test_self(std::string);
void test_self(std::string name) {
  auto content = read_file(name);
  auto st = std::make_shared<Ukkonen<uint8_t>>(content);
  assert(st->max_common_prefix(content) == content.size());
  // st->dfs();
}

size_t strategy_cost(const std::vector<std::pair<bool, size_t>> strategy);
size_t strategy_cost(const std::vector<std::pair<bool, size_t>> strategy) {
  size_t total_size = 0;
  for(auto it: strategy) {
    // std::cout << it.first << " " << it.second << std::endl;
    if(it.first) {
      total_size += complex_overhead;
    } else {
      total_size += fixed_overhead + it.second;
    }
  }
  return total_size;
}

template <typename T>
std::pair<size_t, size_t> search_st(std::shared_ptr<Ukkonen<T>> st, std::vector<T>& target_content) {

  auto be = target_content.begin();
  auto fin = target_content.end();

  std::map<size_t, size_t> sizes;
  auto strr = std::string("std");
  deb(st->max_common_prefix(std::vector<uint8_t>(strr.begin(), strr.end())));
  std::vector<std::pair<bool, size_t>> chunks;
  while(be != fin){
    auto orig = be;
    be = st->max_common_prefix(be, fin);
    auto len = static_cast<size_t>(be - orig);
    if(!len) {
      len += 1;
      deb(*be);
      ++be;
    }
    chunks.emplace_back(true, len);
    sizes[len] += 1;
    // while(orig != be) {
    //   std::cout << static_cast<char>(*orig);
    //   orig++;
    // }
    // std::cout << std::endl;
    // std::cout << "////////////////////////////////////////////////" << std::endl;
    // std::cout << std::endl;
  }

  size_t thres = 3;

  // std::cout << "initial strategy cost: " << strategy_cost(chunks) << std::endl;

  // for(auto& it: sizes) {
  //   std::cout << it.first << " : " << it.second << std::endl;
  // }

  std::vector<std::pair<bool, size_t>> strategy;

  for(auto it: chunks) {
    auto sz = it.second;
    if(!strategy.empty() && !strategy.back().first &&
        sz <= complex_overhead + thres) {
        strategy.back().second += sz;
      } else {
      strategy.emplace_back(
          !(sz+ fixed_overhead <= complex_overhead + thres), sz);
    }
  }

  auto final_cost = strategy_cost(strategy);

  return {final_cost, target_content.size()};

  // std::cout << "strategy cost after merging small chunks: " << final_cost
  //   << std::endl;

  // std::cout << "target size: " << (target_content.size()) << std::endl;
  // std::cout << "compressed/raw: "
  //   << std::setprecision(5) << std::fixed
  //   << ((final_cost + 0.0)/target_content.size()) << std::endl;
}

void handle_batch(std::vector<std::string> targets, std::vector<std::string> bases);
void handle_batch(std::vector<std::string> targets, std::vector<std::string> bases) {
  std::vector<uint8_t> base_content;
  for(auto base_fname: bases) {
    auto content = read_file(base_fname);
    base_content.insert(base_content.end(), content.begin(), content.end());
  }

  auto st = std::make_shared<Ukkonen<uint8_t>>(base_content);

  for(auto target_fname: targets) {
    auto target = read_file(target_fname);
    auto [compressed, actual] = search_st(st, target);
    std::cout << (compressed + 0.0)/actual << std::endl;
  }
}

int main(int argc, char **argv) {

  if(argc == 1) {
    test_uk();
    test_self(argv[0]);
    return 0;
  }

  assert(argc >= 3);

  std::vector<std::string> all_params;
  for(int i = 1; i < argc; i++) {
    all_params.push_back(argv[i]);
  }

  // batch mode
  if(std::find(all_params.begin(), all_params.end(), "--") != std::end(all_params)) {
    std::vector<std::string> target_names, base_names;
    for(int i = 1; i < argc; i++) {
      if(!target_names.empty() && target_names.back() == "--") {
        base_names.push_back(argv[i]);
      } else {
        target_names.push_back(argv[i]);
      }
    }
    target_names.pop_back();

    handle_batch(target_names, base_names);

    return 0;
  }
  std::string target_fname = argv[1];
  std::vector<std::string> bases;
  for(int i = 2; i <argc; i++) {
    bases.push_back(argv[i]);
  }

  std::vector<uint8_t> base_content;
  for(auto base_fname: bases) {
    auto content = read_file(base_fname);
    base_content.insert(base_content.end(), content.begin(), content.end());
  }

  auto target = read_file(target_fname);
  std::vector<uint8_t> target_content(target.begin(), target.end());

  auto st = std::make_shared<Ukkonen<uint8_t>>(base_content);

  auto be = target_content.begin();
  auto fin = target_content.end();

  std::map<size_t, size_t> sizes;
  auto strr = std::string("std");
  deb(st->max_common_prefix(std::vector<uint8_t>(strr.begin(), strr.end())));
  std::vector<std::pair<bool, size_t>> chunks;
  while(be != fin){
    auto orig = be;
    be = st->max_common_prefix(be, fin);
    auto len = static_cast<size_t>(be - orig);
    if(!len) {
      len += 1;
      deb(*be);
      ++be;
    }
    chunks.emplace_back(true, len);
    sizes[len] += 1;
    // while(orig != be) {
    //   std::cout << static_cast<char>(*orig);
    //   orig++;
    // }
    // std::cout << std::endl;
    // std::cout << "////////////////////////////////////////////////" << std::endl;
    // std::cout << std::endl;
  }

  size_t thres = 3;

  std::cout << "initial strategy cost: " << strategy_cost(chunks) << std::endl;

  // for(auto& it: sizes) {
  //   std::cout << it.first << " : " << it.second << std::endl;
  // }

  std::vector<std::pair<bool, size_t>> strategy;

  for(auto it: chunks) {
    auto sz = it.second;
    if(!strategy.empty() && !strategy.back().first &&
        sz <= complex_overhead + thres) {
        strategy.back().second += sz;
      } else {
      strategy.emplace_back(
          !(sz+ fixed_overhead <= complex_overhead + thres), sz);
    }
  }

  auto final_cost = strategy_cost(strategy);

  std::cout << "strategy cost after merging small chunks: " << final_cost
    << std::endl;

  std::cout << "target size: " << (target_content.size()) << std::endl;
  std::cout << "compressed/raw: "
    << std::setprecision(5) << std::fixed
    << ((final_cost + 0.0)/target_content.size()) << std::endl;

  return 0;
}
