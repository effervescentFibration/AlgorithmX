// #! clang++ -Wall -std=c++17 -march=native -o links links.cppo
// g++ -std=c++17 -ggdb -Wall links.cpp -o links
// mv links.cpp links_old.cpp; clang-format links_old.cpp > links.cpp

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class ExactCoverProblem {
public:
  ExactCoverProblem(std::string i, std::vector<std::string> o);
  ExactCoverProblem(ExactCoverProblem &other) = delete;
  ExactCoverProblem(ExactCoverProblem &&other) = delete;
  ExactCoverProblem operator=(ExactCoverProblem &other) = delete;
  ExactCoverProblem operator=(ExactCoverProblem &&other) = delete;
  ~ExactCoverProblem() {}

  void solve();
  std::string solution();
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

  void place_spacer(size_t node_index, size_t option_index);
  void place_node(size_t node_index, size_t item_index);
  size_t choose_item_to_cover();
  void algorithm_x();

  void cover(int64_t i);
  void uncover(int64_t i);
  void hide(int64_t p);
  void unhide(int64_t p);

  std::string items_description;
  std::vector<std::string> options_description;
  std::vector<Item> items;
  std::vector<Node> nodes;
  std::vector<size_t> solution_vector;
};

ExactCoverProblem::ExactCoverProblem(std::string i,
                                     std::vector<std::string> o) {
  // First, copy over the problem description.
  this->items_description = i;
  this->options_description = o;
  // Having copied the description, allocate the intrinstic data structures.
  initialize_problem();
  return;
}

void ExactCoverProblem::initialize_problem() {
  initialize_items();
  initialize_nodes();
  solution_vector.resize(options_description.size());
}

void ExactCoverProblem::initialize_items() {
  // The first item is a header; the rest correspond with the items.
  items.resize(items_description.size() + 1);
  // Inialize the header.
  items[0].name = 0;
  items[0].llink = items.size() - 1;
  items[0].rlink = 1;
  // Initialize the item nodes.
  size_t i = 1;
  for (char item_name : items_description) {
    items[i].name = (int64_t)item_name;
    items[i].llink = i - 1;
    items[i].rlink = (i + 1 != items.size()) ? (i + 1) : 0;

    ++i;
  }
}

void ExactCoverProblem::initialize_nodes() {
  /*
   * First, find the right number of nodes.
   * There will be node to represent each item, one spacer node for each item,
   * and an extra header node. Since the items vector contains one entry for
   * each item plus a header node, double its count and take one away for these.
   */
  size_t node_count = (2 * items.size()) - 1;
  // Now add one node for each presence of an item in each option.
  for (const std::string &option_name : options_description) {
    node_count += option_name.size();
  }

  nodes.resize(node_count);

  /* Now we allocate.
   */

  // i is the index of the node being allocated. We begin with the first node.
  size_t i = 0;
  // Allocate the header.
  ItemNode *item_node = (ItemNode *)&nodes[i];
  item_node->len = 0;
  item_node->ulink = 0;
  item_node->dlink = 0;

  // Allocate one item node for each item.
  for (i = 1; i <= items_description.size(); ++i) {
    item_node = (ItemNode *)&nodes[i];
    /* To begin with, each item is listed in no options, and all its links are
     * self-references. This is essential for how later nodes are then added in.
     */
    item_node->len = 0;
    item_node->ulink = i;
    item_node->dlink = i;
  }

  // Initialize first spacer and move increment the node index.
  nodes[i].top = 0;
  nodes[i].ulink = 0;
  nodes[i].dlink = i + options_description[0].size();
  ++i;

  int64_t option_index = 1;

  for (const std::string &option_name : options_description) {
    size_t item_index = 1;
    for (char option_item : option_name) {
      while (items[item_index].name < (int64_t)option_item) {
        ++item_index;
      }
      place_node(i, item_index);
      ++i;
      ++item_index;
    }
    // Having allocated the option's nodes, allocate a tailing spacer.
    place_spacer(i, option_index);
    ++i;
    ++option_index;
  }
}

void ExactCoverProblem::place_spacer(size_t node_index, size_t option_index) {
  Node *node = &nodes[node_index]; // The node to be set.
  /* Per Knuth (p. 67), ulink will be the index of the last node of the next
   * option; ulink will be the index of the first node of the previous option.
   * Both of these can be determined from the sizes of the respective options.
   */
  /* If this option isn't for the first spacer, have dlink point into nodes
   * for the previous option.
   */
  if (option_index > 0) {
    node->top = -(option_index);
    node->ulink = node_index - options_description[option_index - 1].size();
  }
  // Otherwise, there is no such node. Set the null link.
  else {
    node->top = 0;
    node->ulink = 0;
  }
  /* If this option isn't for the last spacer, have dlink point into nodes
   * for the next option.
   */
  if (option_index < options_description.size() - 1) {
    node->dlink = node_index + options_description[option_index].size();
  }
  // Otherwise, there is no such node. Set the null link.
  else {
    node->dlink = 0;
  }
}

void ExactCoverProblem::place_node(size_t node_index, size_t item_index) {
  ItemNode *item_node =
      (ItemNode *)&nodes[item_index]; // The node for this item.
  Node *node = &nodes[node_index];    // The node to be set.
  if (item_node->ulink == (int64_t)item_index) {
    /* If item_node's dlink is a self-reference, no options have been added to
     * this item before.
     */
    item_node->dlink = node_index;
    node->ulink = item_index;
  } else {
    // Otherwise, there is a regular node above this. Grab it.
    size_t prev_index = item_node->ulink;
    Node *prev_node = &nodes[prev_index];
    prev_node->dlink = node_index;
    node->ulink = prev_index;
  }
  // In any case, the newly emplaced node will be directly above the item
  // node.
  node->dlink = item_index;
  item_node->ulink = node_index;

  // The node's top will be the item.
  node->top = item_index;
  // Also increment the length of item.
  ++(item_node->len);
}

void ExactCoverProblem::cover(int64_t i) {
  int64_t p = nodes[i].dlink;
  while (p != i) {
    hide(p);
    p = nodes[p].dlink;
  }
  int64_t l = items[i].llink;
  int64_t r = items[i].rlink;
  items[l].rlink = r;
  items[r].llink = l;
}

void ExactCoverProblem::uncover(int64_t i) {
  int64_t l = items[i].llink;
  int64_t r = items[i].rlink;
  items[l].rlink = i;
  items[r].llink = i;
  int64_t p = nodes[i].ulink;
  while (p != i) {
    unhide(p);
    p = nodes[p].ulink;
  }
}

void ExactCoverProblem::hide(int64_t p) {
  int64_t q = p + 1;
  while (q != p) {
    int64_t x = nodes[q].top;
    int64_t u = nodes[q].ulink;
    int64_t d = nodes[q].dlink;
    if (x <= 0) {
      // q was a spacer
      q = u;
    } else {
      nodes[u].dlink = d;
      nodes[d].ulink = u;
      // x has one fewer node.
      ItemNode *item_x = (ItemNode *)&nodes[x];
      --(item_x->len);
      if ((item_x->len) < 0)
        while (true)
          ;
      ++q;
    }
  }
}

void ExactCoverProblem::unhide(int64_t p) {
  int64_t q = p - 1;
  while (q != p) {
    int64_t x = nodes[q].top;
    int64_t u = nodes[q].ulink;
    int64_t d = nodes[q].dlink;
    if (x <= 0) {
      // q was a spacer
      q = d;
    } else {
      nodes[u].dlink = q;
      nodes[d].ulink = q;
      // x has one more node.
      ItemNode *item_x = (ItemNode *)&nodes[x];
      ++(item_x->len);
      --q;
    }
  }
}

void ExactCoverProblem::solve() { algorithm_x(); }

std::string ExactCoverProblem::solution() {
  std::basic_stringstream<char> ss;
  for (size_t i = 0; i < solution_vector.size(); ++i) {
    if (solution_vector[i] == 0) {
      continue;
    }
    size_t option_index = -(nodes[solution_vector[i]].top) - 1;
    ss << options_description[option_index];
  }
  return ss.str();
}

void ExactCoverProblem::algorithm_x() {
  /*
   * This is an implementation of Donald Knuth's Algorithm X
   * as posed in _The Art of Computer Programming_,
   * volume 4, fascicle 5 (p. 67). It's a fairly straightforward
   * translation of the pseudocode into idiomatic C++.
   */

  /* X1
   * Initialize.
   */
  // x1:
  size_t n = items_description.size();
  size_t z = nodes.size();
  size_t l = 0;
  size_t p;
  size_t j;
  size_t i;
  goto x2;

  /* X2
   * Enter level l.
   */
x2:
  if (items[0].rlink == 0) {
    // All items have been covered.
    goto x8;
  }

  /* X3
   * Choose i.
   * At this point, the times i_1, ..., i_t still need to be covered, where i_1
   * = items[0].rlink, ..., i_j+1 = items[j].rlink, i_t = 0. Choose one of them,
   * and call it i.
   */
  // x3:
  i = choose_item_to_cover();

  /* X4
   * Cover i.
   */
  // x4:
  cover(i);
  solution_vector[l] = nodes[i].dlink;

  /* X5
   * Try x_l.
   */
x5:
  if (solution_vector[l] == i) {
    // We've tried all options for i.
    while (true)
      ;
    goto x7;
  } else {
    p = solution_vector[l] + 1;
    while (p != solution_vector[l]) {
      j = nodes[solution_vector[l]].top;
      if (j <= 0) {
        p = nodes[p].ulink;
      } else {
        // Cover the items != i in the option that contains x + l.
        cover(j);
        ++p;
      }
    }
    // Now increment l and deepen a level.
    ++l;
    goto x2;
  }

  /* X6
   * Try again.
   */
x6:
  p = solution_vector[l] - 1;
  while (p != solution_vector[l]) {
    j = nodes[p].top;
    if (j <= 0) {
      p = nodes[p].dlink;
    } else {
      uncover(j);
      --p;
    }
  }
  i = nodes[solution_vector[l]].top;
  solution_vector[l] = nodes[solution_vector[l]].dlink;
  goto x5;

  /* X7
   * Backtrack.
   */
x7:
  uncover(i);
  while (true)
    ;
  /* X8
   * Exit level l.
   */
x8:
  if (l == 0) {
    return;
  }
  --l;
  goto x6;
}

/*
 * Use the MRV heuristic from exercise 8.
 */
size_t ExactCoverProblem::choose_item_to_cover() {
  int64_t shortest = INT64_MAX; // Start at the top of the lattice.
  int64_t shortest_index = -1;
  int64_t i = items[0].rlink;
  do {
    ItemNode *item_node = (ItemNode *)&nodes[i];
    if (item_node->len > 0 && item_node->len < shortest) {
      shortest = item_node->len;
      shortest_index = i;
    }
    i = items[i].rlink;
  } while (i != 0);
  return shortest_index;
}

std::string ExactCoverProblem::to_aocp_table() {
  std::basic_stringstream<char> ss;
  size_t item_count = items.size();
  size_t node_count = nodes.size();
  ss << "Items: " << item_count << "\n";
  ss << "Nodes: " << node_count << "\n";
  ss << "i:"
     << "\t\t";
  for (size_t i = 0; i < item_count; ++i) {
    ss << i << "\t";
  }
  ss << "\n";

  ss << "NAME(i):"
     << "\t";
  for (const Item &item : items) {
    if (char(item.name) < 'a') {
      ss << item.name << "\t";
    } else {
      ss << char(item.name) << "\t";
    }
  }
  ss << "\n";

  ss << "LLINK(i):"
     << "\t";
  for (const Item &item : items) {
    ss << item.llink << "\t";
  }
  ss << "\n";

  ss << "RLINK(i):"
     << "\t";
  for (const Item &item : items) {
    ss << item.rlink << "\t";
  }
  ss << "\n";

  size_t row_count = ((node_count - 1) / item_count) + 1;
  for (size_t row = 0; row < row_count; ++row) {
    /*
     * The bound determines how many nodes to place in the row.
     * It's just the item count in all but the last row, which may be
     * incomplete. If the last row is incomplete, it is the those nodes
     * remaining.
     */
    size_t bound = item_count;
    if (row == (row_count - 1)) {
      bound = node_count % item_count;
    }

    ss << "x:"
       << "\t\t";
    for (size_t i = 0; i < bound; ++i) {
      size_t x = i + (row * item_count);
      ss << x << "\t";
    }
    ss << "\n";

    if (row == 0) {
      ss << "LEN(x):"
         << "\t\t";
      for (size_t i = 0; i < bound; ++i) {
        size_t x = i + (row * item_count);
        ItemNode *item_node = (ItemNode *)&nodes[x];
        ss << item_node->len << "\t";
      }
    } else {
      ss << "TOP(x):"
         << "\t\t";
      for (size_t i = 0; i < bound; ++i) {
        size_t x = i + (row * item_count);
        ss << nodes[x].top << "\t";
      }
    }
    ss << "\n";

    ss << "ULINK(x):"
       << "\t";
    for (size_t i = 0; i < bound; ++i) {
      size_t x = i + (row * item_count);
      ss << nodes[x].ulink << "\t";
    }
    ss << "\n";

    ss << "DLINK(x):"
       << "\t";
    for (size_t i = 0; i < bound; ++i) {
      size_t x = i + (row * item_count);
      ss << nodes[x].dlink << "\t";
    }
    ss << "\n";
  }
  return ss.str();
}

int main(int argc, char *argv[]) {
  ExactCoverProblem p{"abcdefg", std::vector<std::string>{"ce", "adg", "bcf",
                                                          "adf", "bg", "deg"}};
  std::cout << p.to_aocp_table();
  std::cout << "Solving...\n";
  p.solve();
  std::cout << "Solved!\n";
  std::cout << p.solution();
  return 0;
}
