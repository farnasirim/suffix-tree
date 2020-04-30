#ifndef ST_UKKONEN_H_
#define ST_UKKONEN_H_

#include <assert.h>

#include <memory>
#include <map>
#include <vector>
#include <limits>
#include <iostream>
#include <sstream>

#include "util.h"

#include "debug.h"


// Maybe inherit from enable_shared_from_this ?
template<typename T>
class UkkonenNode {
  using NodeP = std::shared_ptr<UkkonenNode>;
 public:
  UkkonenNode(int from, int to) noexcept: from_(from), to_(to) { }

  NodeP get_child(T ch) const {
    auto it = children_.find(ch);
    if(it != children_.end()) {
      return it->second;
    }
    assert(false);
  }

  bool is_leaf() const {
    return children_.empty();
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
    assert(link_ != nullptr);
    return link_;
  }

  ssize_t get_from() const {
    return from_;
  }

  ssize_t get_to() const {
    return to_;
  }

  void set_from(ssize_t from) {
    from_ = from;
  }

  void set_to(ssize_t to) {
    to_ = to;
  }

  // what string does the edge incoming to this node represent? [from, to]
 private:
  ssize_t from_;
  ssize_t to_;
  std::map<T, NodeP> children_;
  NodeP link_ = nullptr;
};

/**
 *  |
 *  |
 * In temporal order
 *  |
 *  |
 *  v
 *
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
 * Explicit states: Those that branch out + laves
 *
 * Generalized transition function: g(state, some_str), or g(state, (s, t))
 *   Such that input_str[s..t] == some_str
 *
 * New suffix link:
 *   only defined for branching states x. f'(x) = y, and y will also be a
 *   branching state.
 * Implicit sufix links: imaginary suffix links between implicit states.
 *
 * Canonical representation of an implicit state: (s, w), where s is the
 * lowest (explicit) ancestor of the string (and implicit state) sw.
 *
 * To be able to search (and locate) in substrings of S, we need to keep the
 * the starting index of each suffix in its node.
 *
 * When inserting, think about what the canonical path looks like:
 * At some point, the nodes stop being leaves (j = last leaf),
 * and at some point, it starts having ti transitions
 * (j': first node with ti transition).
 *
 * Now when adding a character, for every node from =1 to <j, we only
 * expand the incoming edge to a node, and for =j to <j', we add a branch.
 * Sj: the active point (actively adding branches) , Sj' (end point)
 *
 * State (S, eps) is represented with (S, (p + 1, p)), where p is the index
 * of the last character of S.
 *
 * First group of transactions are handled with "open transitions":
 * Any edge to a node with zero children is open to be expanded.
 *
 * Nodes on the boundary path are suffixes to Trie_i-1, therefore their "to"
 * index is i - 1. If we're not yet at the endpoint, we have to branch off. If
 * this (s, w) or (s, (k, i-1)) is not explicit, we have to make it explicit:
 *   split the transition: we have to set the suffix link for the new node too.
 *   New ti transition is an open transition, leading to a new leaf.
 * next node to be updated is f((s, w)), canonized. (right pointer is unchanged
 * during canonize: everything that is canonized is a suffix of T_i-1).
 * Second pointer remains i - 1 for everyone along the boundary path.
 *
 * Active point of T_i-1: longest suffix that occured at least twice in T_i-1
 * End point of T_i-1: longest suffix that concat(t1..ti-1, ti) is a substring
 * in T_i-1. Therefore g(endpoint_i-1, ti) is the active point of i.
 */
template <typename T>
class Ukkonen {
  using Node = UkkonenNode<T>;
  using NodeP = std::shared_ptr<Node>;

 public:
  Ukkonen(const std::vector<T> &str)
      : str_(str),
        falsum_(std::make_shared<Node>(-1, -1)),
        root_(std::make_shared<Node>(-1, -1)) {

    // falsum_->root = root_;
    auto st = std::numeric_limits<T>::min();
    auto mx = std::numeric_limits<T>::max();
    while(true) {
      falsum_->set_child(st, root_);
      if(st == mx) {
        break;
      }
      ++st;
    }

    root_->set_link(falsum_);

    NodeP active_point = root_;
    size_t left_ptr = 0;

    inf_ = str_.size() - 1;

    for(size_t i = 0; i < str_.size(); i++) {
      debout("main loop: calling update");
      {
        auto [pt, lef] = update(active_point, left_ptr, i);
        active_point = pt;
        left_ptr = lef;
      }
      debout("main loop: calling canonize");
      {
        auto [pt, lef] = canonize(active_point, left_ptr, i);
        active_point = pt;
        left_ptr = lef;
      }
    }
  }

  // test and split
  std::pair<bool, NodeP> split_or_is_endpoint(const NodeP& node,
      size_t lef, size_t rig, T ch) {
    deb(lef);
    deb(rig);

    if(lef > rig) { // (node, eps): node is already explicit
      return {node->has_child(ch), node};
    }

    auto corresp_child = node->get_child(str_[lef]);

    // If character ch found along the path:
    auto child_fr = corresp_child->get_from();
    assert(child_fr >= 0);
    auto child_fr_ind = static_cast<size_t>(child_fr);
    assert_between_close_open(
        static_cast<long long>(0),
        static_cast<long long>(child_fr_ind + rig - lef + 1),
        static_cast<long long>(str_.size()));
    if(ch == str_[child_fr_ind + rig - lef + 1]) {
      return {true, node};
    }

    auto new_state = std::make_shared<Node>(
        corresp_child->get_from(),
        child_fr_ind + rig - lef);

    new_state->set_child(child_fr, corresp_child);
    corresp_child->set_from(static_cast<ssize_t>(child_fr_ind + rig - lef + 1));

    node->set_child(str_[lef], new_state);

    return {false, new_state};
  }

  // returns active_point_, left_ptr_
  std::pair<NodeP, size_t> update(NodeP active_point,
      size_t left_ptr, size_t ind) {

    auto ch = str_[ind];

    // (active_point_, (left_ptr_, ind - 1)) is the canonical active point
    auto oldr = root_;
    auto split_res = split_or_is_endpoint(
        active_point, left_ptr, ind - 1, ch);

    auto is_endpoint = split_res.first;
    auto operating_node = split_res.second;

    while(!is_endpoint) {
      auto new_state = std::make_shared<Node>(ind, inf_);
      operating_node->set_child(ch, new_state);

      if(oldr != root_) {
        oldr->set_link(operating_node);
      }
      oldr = operating_node;

      auto [pt, lef] = canonize(
          active_point->get_link(), left_ptr, ind - 1);
      active_point = pt;
      left_ptr = lef;

      split_res = split_or_is_endpoint(active_point, left_ptr, ind - 1, ch);
      is_endpoint = split_res.first;
      operating_node = split_res.second;
    }

    if(oldr != root_) {
      oldr->set_link(active_point);
    }
    return {active_point, left_ptr};
  }

  // operates on active_point_, left_ptr_, ind
  std::pair<NodeP, size_t> canonize(NodeP node, size_t left, size_t right) {
    return {nullptr, 0};
  }

  size_t inf_;

  //void insert(T ch) {
  //  NodeP r = top_;
  //  // deb(top_.get());
  //  // deb(r.get());
  //  assert(r == top_);
  //  NodeP old_child = nullptr;

  //  assert(r != nullptr);
  //  // deb(falsum_.get());
  //  // deb(root_.get());
  //  // debline();
  //  while(!r->has_child(ch)) {
  //    // deb(r == root_);
  //    auto child = std::make_shared<Node>();
  //    // deb(child.get());

  //    assert(r != falsum_);
  //    r->set_child(ch, child);
  //    if(r != top_) {
  //      assert(child != old_child);
  //      assert(old_child != falsum_);
  //      // debout("set_link 1");
  //      // deb(old_child.get());
  //      // deb(child.get());
  //      old_child->set_link(child);
  //    }
  //    old_child = child;

  //    // deb(r.get());
  //    // deb(r->get_link().get());
  //    r = r->get_link();
  //  }
  //  // debline();

  //  // deb(r == falsum_);
  //  assert(old_child != r->get_child(ch));
  //  // debout("set_link 2");
  //  // deb(old_child.get());
  //  // deb(r->get_child(ch));
  //  // deb(r.get());
  //  // deb(ch);
  //  old_child->set_link(r->get_child(ch));
  //  top_ = top_->get_child(ch);
  //  // deb(top_ == root_);
  //  // deb(top_ == root_->get_child(ch));
  //  assert(top_ != nullptr);
  //  assert(r != nullptr);
  //}

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

    int rem = current->get_children_ref().size();
    for(auto& [ch, child]: current->get_children_ref()) {
      std::stringstream ss;
      if(child->get_from()<= child->get_to()) {
        for(ssize_t i = child->get_from(); i <= child->get_to(); i++) {
          assert(i > 0);
          ss << str_[static_cast<size_t>(i)];
        }
      } else {
        ss << "EPS" << std::endl;
      }
      auto num_spaces = ss.str().size();
      std::cout << indent << bef << ss.str() << aft;
      rem -= 1;
      if(rem == 0) {
      indent.back() = ' ';
      }
      dfs(child,
          indent +
          std::string(num_spaces + bef.size() + aft.size(), ' ')
         );
    }
    indent.pop_back();
    std::cout << indent << std::endl;
  }

  std::vector<T> str_;

  NodeP falsum_;
  NodeP root_;
};

#endif  // ST_UKKONEN_H_
