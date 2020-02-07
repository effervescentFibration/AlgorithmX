#include "algorithm_x.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace algorithm_x {

ExactCoverProblem::ExactCoverProblem(std::string i,
                                     std::vector<std::string> o) {
  has_string_description = true;
  // First, copy over the items description.
  items_description.reserve(i.size());
  for (char c : i) {
    items_description.push_back(c);
  }

  // Next, copy over the options description.
  options_description.reserve(o.size());
  for (const std::string &option_name : o) {
    options_description.push_back({});
    std::vector<int64_t> &option = options_description.back();
    for (char c : option_name) {
      option.push_back(c);
    }
  }

  // Having copied the problem description, allocate the intrinsic data
  // structures.
  initialize_problem();
  return;
}

ExactCoverProblem::ExactCoverProblem(std::vector<int64_t> i,
                                     std::vector<std::vector<int64_t>> o) {
  has_string_description = false;
  // First, copy over the problem description.
  this->items_description = i;
  this->options_description = o;

  // Having copied the description, allocate the intrinsic data structures.
  initialize_problem();
  return;
}

ExactCoverProblem::ExactCoverProblem(ExactCoverProblem &other) = default;

ExactCoverProblem::ExactCoverProblem(ExactCoverProblem &&other) = default;

ExactCoverProblem &ExactCoverProblem::
operator=(ExactCoverProblem &other) = default;

ExactCoverProblem &ExactCoverProblem::
operator=(ExactCoverProblem &&other) = default;

ExactCoverProblem::~ExactCoverProblem() {}

void ExactCoverProblem::initialize_problem() {
  // The problem is initialized in an unsolved state.
  solved = false;
  initialize_items();
  initialize_nodes();
  candidate.reserve(options_description.size());
}

void ExactCoverProblem::initialize_items() {
  // The first item is a header; the rest correspond with the items.
  items.resize(items_description.size() + 1);
  // Inialize the header.
  items[0].name = 0;
  items[0].llink = items.size() - 1;
  items[0].rlink = 1;
  // Initialize the item nodes.
  int64_t i = 1;
  for (int64_t item_name : items_description) {
    items[i].name = item_name;
    items[i].llink = i - 1;
    items[i].rlink = ((int64_t)items.size() != i + 1) ? (i + 1) : 0;
    ++i;
  }
}

void ExactCoverProblem::initialize_nodes() {
  /*
   * First, find the right number of nodes. This count is based on the diagram
   * shown in Knuth (p.66).
   *
   * In the option items list, there will be one header node for each item after
   * a spacer. This is equal to the total number of item nodes (which is one
   * more than the total number of items. There will also be one space for each
   * option, one node for each item's presence in an option, and one final
   * spacer node.
   *
   * This brings the total allocated node count to:
   *   (# item nodes) + (# options) + (# item options) + 1
   * The logic below simply carries this calculation out.
   */

  int64_t node_count = items.size() + options_description.size() + 1;
  for (const std::vector<int64_t> &option_name : options_description) {
    node_count += option_name.size();
  }

  nodes.resize(node_count);

  /* Now we allocate.
   */

  // i is the index of the node being allocated. We begin with the first node.
  int64_t i = 0;
  // Allocate the header.
  ItemNode *item_node = (ItemNode *)&nodes[i];
  item_node->len = 0;
  item_node->ulink = 0;
  item_node->dlink = 0;

  // Allocate one item node for each item.
  for (i = 1; i <= (int64_t)items_description.size(); ++i) {
    item_node = (ItemNode *)&nodes[i];
    /* To begin with, each item is listed in no options, and all its links are
     * self-references. This is essential for how later nodes are then added in.
     */
    item_node->len = 0;
    item_node->ulink = i;
    item_node->dlink = i;
  }

  // Initialize first spacer and increment the node index.
  nodes[i].top = 0;
  nodes[i].ulink = 0;
  nodes[i].dlink = i + options_description[0].size();
  ++i;

  int64_t option_index = 1;

  for (const std::vector<int64_t> &option_name : options_description) {
    int64_t item_index = 1;
    for (int64_t option_item : option_name) {
      while (items[item_index].name < option_item) {
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

void ExactCoverProblem::place_spacer(int64_t node_index, int64_t option_index) {
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
  if (option_index < (int64_t)options_description.size()) {
    node->dlink = node_index + options_description[option_index].size();
  }
  // Otherwise, there is no such node. Set the null link.
  else {
    node->dlink = 0;
  }
}

void ExactCoverProblem::place_node(int64_t node_index, int64_t item_index) {
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
    int64_t prev_index = item_node->ulink;
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
  (item_node->len) += 1;
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
      // x has one less node.
      ItemNode *item_x = (ItemNode *)&nodes[x];
      --(item_x->len);
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

void ExactCoverProblem::solve(bool find_all_solutions) {
  if (!solved) {
    algorithm_x(find_all_solutions);
    solved = true;
  }
}

void ExactCoverProblem::algorithm_x(bool find_all_solutions) {
  /*
   * This is an implementation of Donald Knuth's Algorithm X
   * as posed in _The Art of Computer Programming_,
   * volume 4, fascicle 5 (p. 67). It's a fairly straightforward
   * translation of the pseudocode into idiomatic C++. In particular, it
   * foregoes recursion, structured control flow or any inversions of the same.
   */

  /* X1
   * Initialize.
   */
  // x1:
  int64_t l = 0;
  int64_t p;
  int64_t j;
  int64_t i;
  goto x2;

  /* X2
   * Enter level l.
   */
x2:
  if (items[0].rlink == 0) {
    // All items have been covered.
    append_solution();

    /*
     * Here we deviate from Knuth. If find_all_solutions is false, then we
     * return right now, having found a solution. Otherwise, we jump to X8, as
     * in Knuth.
     */
    if (!find_all_solutions)
      return;

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
  candidate.push_back(nodes[i].dlink);
  goto x5;

  /* X5
   * Try x_l.
   */
x5:
  if (candidate[l] == i) {
    /* We've tried all options for i to no avail. We must backtrack. */
    goto x7;
  } else {
    p = candidate[l] + 1;
    while (p != candidate[l]) {
      j = nodes[p].top;

      if (j <= 0) {
        // This is a spacer
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
  p = candidate[l] - 1;
  while (p != candidate[l]) {
    j = nodes[p].top;
    if (j <= 0) {
      p = nodes[p].dlink;
    } else {
      uncover(j);
      --p;
    }
  }
  i = nodes[candidate[l]].top;
  candidate[l] = nodes[candidate[l]].dlink;
  goto x5;

  /* X7
   * Backtrack.
   */
x7:
  uncover(i);

  /* X8
   * Exit level l.
   */
x8:
  if (l == 0) {
    return;
  }
  candidate.pop_back();
  --l;
  goto x6;
}

/* append_solution() stores a vector of strings, each representing an option
 * chosen in the solution. Each option is represented in accordance with
 * exercise 12 (p. 123), where the representation is rotated to the left such
 * that the item that led to that option being chosen is printed first. For
 * instance, if the item "d" led to the option "adf" being chosen, this choice
 * would be represented as "dfa".
 */
void ExactCoverProblem::append_solution() {
  solutions.push_back({});
  std::vector<std::vector<int64_t>> &solution = solutions.back();

  for (int64_t rep_index : candidate) {
    /* Get the index of the item this representative refers to so we
     * can find the item that led to this choice of option. */
    int64_t item_index = nodes[rep_index].top;
    Item *item = (Item *)&nodes[item_index];
    int64_t item_name = item->name;

    /* The first spacer to follow this node will have a top the negative of
     * which is the index of the first option in the representation. Find it by
     * incrementing rep_index until the spacer is found.
     */
    while (nodes[rep_index].top > 0) {
      --rep_index;
    }

    int64_t option_index = -(nodes[rep_index].top);
    std::vector<int64_t> *option_name = &options_description[option_index];

    // We write out the option as led by the representative item into
    // option_rep.
    std::vector<int64_t> option_rep{};
    int64_t offset = 0; /* the offset represents the index of the representative
                           item in the original option representation */
    for (int64_t option_item : *option_name) {
      if (option_item == item_name) {
        break;
      }
      ++offset;
    }
    /* Having found the offset, cycle through the original option representation
     * starting with the representative item. */
    int64_t option_size = option_name->size();
    for (int64_t i = 0; i < option_size; ++i) {
      int64_t item = (*option_name)[(i + offset) % option_size];
      option_rep.push_back(item);
    }
    solution.push_back(std::move(option_rep));
  }
}

const std::string
ExactCoverProblem::option_str(const std::vector<int64_t> &option) const {
  std::stringstream ss;
  if (has_string_description) {
    for (int64_t i : option) {
      ss << (char)i;
    }
  } else {
    for (int64_t i = 0; i < (int64_t)option.size(); ++i) {
      if (i > 0) {
        // We separate item IDs with commas.
        ss << ':';
      }
      ss << option[i];
    }
  }
  return ss.str();
}

const std::vector<std::vector<std::vector<int64_t>>> &
ExactCoverProblem::get_solutions() const {
  return solutions;
}

const std::string ExactCoverProblem::solutions_string() const {
  // Exit early for an empty solutions vector.
  if (solutions.size() == 0) {
    return ("The solution set is empty. "
            "Either it has no solution, "
            "or you never invoked solve().");
  }

  std::basic_stringstream<char> ss;
  /* We will comma separate solutions and the options in each solution. Indices
   * are used to track these.
   */
  int64_t i = 0;
  for (const std::vector<std::vector<int64_t>> &solution_vec : solutions) {
    // If this isn't the first solution, add a comma to separate it.
    if (i > 0) {
      ss << ", ";
    }
    ss << "{";
    int64_t j = 0;
    for (const std::vector<int64_t> &option : solution_vec) {
      // If this isn't the first option, add a comma to separate it.
      if (j > 0) {
        ss << ", ";
      }
      ss << option_str(option);
      ++j;
    }
    ss << "}";
    ++i;
  }
  return ss.str();
}

/*
 * For this implementation, we use the MRV (minimum remaining values) heuristic
 * from exercise 9 (p. 123).
 */
int64_t ExactCoverProblem::choose_item_to_cover() {
  int64_t shortest = INT64_MAX; // Start at the top of the lattice.
  int64_t shortest_index = -1;
  int64_t i = items[0].rlink;
  while (i != 0) {
    ItemNode *item_node = (ItemNode *)&nodes[i];
    if (item_node->len < shortest) {
      shortest = item_node->len;
      shortest_index = i;
    }
    i = items[i].rlink;
  }
  return shortest_index;
}

/* This method outputs a table formatted like Table 1 in The Art of Computer
 * Programming, volume 4, fascicle 5 (p. 66). It's useful both for debugging and
 * for better understanding the functioning of the algorithm. */
const std::string ExactCoverProblem::to_aocp_table() const {
  std::basic_stringstream<char> ss;
  int64_t item_count = items.size();
  int64_t node_count = nodes.size();
  ss << "Items: " << item_count << "\n";
  ss << "Nodes: " << node_count << "\n";
  ss << "i:"
     << "\t\t";
  for (int64_t i = 0; i < item_count; ++i) {
    ss << i << "\t";
  }
  ss << "\n";

  ss << "NAME(i):"
     << "\t";
  int64_t i = 0;
  for (const Item &item : items) {
    // The first item slot is always null.
    // Don't output it as a character.
    if (has_string_description && i > 0) {
      ss << char(item.name) << "\t";
    } else {
      ss << item.name << "\t";
    }
    ++i;
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

  int64_t row_count = ((node_count - 1) / item_count) + 1;
  for (int64_t row = 0; row < row_count; ++row) {
    /*
     * The bound determines how many nodes to place in the row.
     * It's just the item count in all but the last row, which may be
     * incomplete. If the last row is incomplete, it is just those nodes
     * remaining.
     */
    int64_t bound = item_count;
    if (row == (row_count - 1) && node_count % item_count) {
      bound = node_count % item_count;
    }

    ss << "x:"
       << "\t\t";
    for (int64_t i = 0; i < bound; ++i) {
      int64_t x = i + (row * item_count);
      ss << x << "\t";
    }
    ss << "\n";

    if (row == 0) {
      ss << "LEN(x):"
         << "\t\t";
      for (int64_t i = 0; i < bound; ++i) {
        int64_t x = i + (row * item_count);
        ItemNode *item_node = (ItemNode *)&nodes[x];
        ss << item_node->len << "\t";
      }
    } else {
      ss << "TOP(x):"
         << "\t\t";
      for (int64_t i = 0; i < bound; ++i) {
        int64_t x = i + (row * item_count);
        ss << nodes[x].top << "\t";
      }
    }
    ss << "\n";

    ss << "ULINK(x):"
       << "\t";
    for (int64_t i = 0; i < bound; ++i) {
      int64_t x = i + (row * item_count);
      ss << nodes[x].ulink << "\t";
    }
    ss << "\n";

    ss << "DLINK(x):"
       << "\t";
    for (int64_t i = 0; i < bound; ++i) {
      int64_t x = i + (row * item_count);
      ss << nodes[x].dlink << "\t";
    }
    ss << "\n";
  }
  return ss.str();
}

} // namespace algorithm_x
