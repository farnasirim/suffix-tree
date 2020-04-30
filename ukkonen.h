#ifndef ST_UKKONEN_H_
#define ST_UKKONEN_H_

#include <assert.h>

#include <memory>
#include <map>
#include <vector>
#include <limits>
#include <iostream>
#include <sstream>

#include "debug.h"


// Maybe inherit from enable_shared_from_this ?
template<typename T>
class UkkonenNode {
  using NodeP = std::shared_ptr<UkkonenNode>;
 public:
  UkkonenNode() noexcept { }

  NodeP get_child(T ch) const {
    auto it = children_.find(ch);
    if(it != children_.end()) {
      return it->second;
    }
    assert(false);
  }

  void set_child(T ch, NodeP child) {
    children_[ch] = child;
  }

  const std::map<T, NodeP>& get_children_ref() const {
    return children_;
  }

  bool has_child(T next) const {
    return children_.find(next) != children_.end();
  }

  void set_link(NodeP link) {
    link_ = link;
  }

  NodeP get_link() const {
    return link_;
  }

 private:
  std::map<T, NodeP> children_;
  NodeP link_;
};

/**
 * g(x, a): where does a (g)o from node x
 * f(x): takes away the (f)irst character in x
 *
 * f(falsum): undefined
 * g(falsum, all): root
 * f(root): falsum
 * :: falsum consitently acts as inverse of all symbols.
 *
 * f(r) is called the suffix link of state r.
 *
 * Boundary path: path from the deepest state to falsum, following suffix
 * links.
 *   -> If some node x on the boundary path doesn't have the transition
 *      required for the new character c: g(x, c) -> new node.
 *   -> For f's: the new nodes are linked together, forming the boundary path
 *      of the new trie.
 *   -> The first node to have g(node, c) defined, guarantees that all
 *      f^n(node)'s also have g(f^n(node), c) defined. Also this thing always
 *      exists: g(falsum, all) -> root
 *
 */
template <typename T>
class Ukkonen {
  using Node = UkkonenNode<T>;
  using NodeP = std::shared_ptr<Node>;

 public:
  Ukkonen(const std::vector<T> &str)
      : str_(str),
        falsum_(std::make_shared<Node>()),
        root_(std::make_shared<Node>()),
        top_(root_) {

    auto st = std::numeric_limits<T>::min();
    auto mx = std::numeric_limits<T>::max();
    while(true) {
      falsum_->set_child(st, std::make_shared<Node>());
      if(st == mx) {
        break;
      }
      ++st;
    }

    root_->set_link(falsum_);

    for (auto ch : str_) {
      deb(ch);
      insert(ch);
    }

  }

  void insert(T ch) {
    auto r = top_;
    NodeP old_child = nullptr;

    assert(r != nullptr);
    while(!r->has_child(ch)) {
      deb(r == root_);
      auto child = std::make_shared<Node>();
      r->set_child(ch, child);
      if(r != top_) {
        assert(child != old_child);
        old_child->set_link(child);
      }
      old_child = child;
      r = r->get_link();
    }
    deb(r == falsum_);
    assert(old_child != r->get_child(ch));
    old_child->set_link(r->get_child(ch));
    top_ = top_->get_child(ch);
    deb(top_ == root_);
    deb(top_ == root_->get_child(ch));
    assert(top_ != nullptr);
    assert(r != nullptr);
  }

  void dfs() const {
    dfs(root_, "");
  }

 private:

  void dfs(NodeP current, std::string indent) const {
    if(current->get_children_ref().empty()) {
      std::cout << "/" << std::endl;
      return;
    }

    std::cout << "O" << std::endl;

    indent += "|";

    std::string bef = "-- ";
    std::string aft = " --> ";

    for(auto& char_child: current->get_children_ref()) {
      std::stringstream ss;
      ss << char_child.first;
      auto num_spaces = ss.str().size();
      std::cout << indent << bef << ss.str() << aft;
      dfs(char_child.second,
          indent +
          std::string(num_spaces + bef.size() + aft.size(), ' ')
         );
    }
  }

  std::vector<T> str_;

  NodeP falsum_;
  NodeP root_;

  NodeP top_;
};

#endif  // ST_UKKONEN_H_
