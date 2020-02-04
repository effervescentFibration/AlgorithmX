#include "algorithm_x.h"
#include "langford_pairs.h"
#include "xc_equivalent_problem.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

/*
 * This entry point is offered as an example of how the ExactCoverProblem class
 * can be used, recreating the problem put forth in _The Art of Computer
 * Programming_, volume 4, fascicle 5 ((6), p. 64).
 */
int main(int argc, char *argv[]) {
  algorithm_x::ExactCoverProblem p{
      "abcdefg",
      std::vector<std::string>{"ce", "adg", "bcf", "adf", "bg", "deg"}};
  std::cout << "Art of Computer Programming-style input table:\n";
  std::cout << p.to_aocp_table();
  std::cout << "Solving...\n";
  p.solve();
  std::cout << "Solved! Here is the solution set "
               "(each solution given as a set):\n";
  std::cout << p.solutions_string() << '\n';

  algorithm_x::LangfordPairsProblem lp{4};
  lp.solve(false);
  std::cout << "Solved Langford Pairs problem for n = 7! Here is the solution set "
               "(each solution given as a set):\n";
  std::cout << lp.get_exact_cover_problem().solutions_string() << '\n';
  
  return 0;
}
