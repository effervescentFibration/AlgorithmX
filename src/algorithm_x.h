#ifndef ALGORITHM_X_H
#define ALGORITHM_X_H

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace algorithm_x {

class ExactCoverProblem {
public:
  ExactCoverProblem(std::string i, std::vector<std::string> o);
  ExactCoverProblem(ExactCoverProblem &other);
  ExactCoverProblem(ExactCoverProblem &&other);
  ExactCoverProblem &operator=(ExactCoverProblem &other);
  ExactCoverProblem &operator=(ExactCoverProblem &&other);
  ~ExactCoverProblem();

  void solve();
  std::string solutions_string();
  std::string to_aocp_table();

private:
  struct Item {
    int64_t name;
    int64_t llink;
    int64_t rlink;
  };

  struct ItemNode {
    int64_t len;
    int64_t ulink;
    int64_t dlink;
  };

  struct Node {
    int64_t top;
    int64_t ulink;
    int64_t dlink;
  };

  void initialize_problem();
  void initialize_items();
  void initialize_nodes();

  void place_spacer(int64_t node_index, int64_t option_index);
  void place_node(int64_t node_index, int64_t item_index);
  int64_t choose_item_to_cover();
  void algorithm_x();
  void append_solution();

  void cover(int64_t i);
  void uncover(int64_t i);
  void hide(int64_t p);
  void unhide(int64_t p);

  std::string items_description;
  std::vector<std::string> options_description;
  std::vector<Item> items;
  std::vector<Node> nodes;
  std::vector<int64_t> candidate;
  std::vector<std::vector<std::string>> solutions;
};

} // namespace algorithm_x

#endif // #define ALGORITHM_X_H
