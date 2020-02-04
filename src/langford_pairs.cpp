#include "langford_pairs.h"
#include "algorithm_x.h"
#include <cstdint>
#include <stdexcept>
#include <string>

namespace algorithm_x {
LangfordPairsProblem::LangfordPairsProblem(int64_t n) {
  if ((n - 1) > INT64_MAX / 3) {
    throw std::range_error("Problem instance is too large "
                           "for exact cover problem solver.");
  }
  this->n = n;
  initialize();
  this->exact_cover_problem = new ExactCoverProblem(items, options);
  return;
}

void LangfordPairsProblem::initialize() {
  /* We represent the n different numbers (each of which appears twice in the
   * problem) with the first n items of the exact cover problem and the 2*n
   * different slots with the last 2*n items.
   */
  for (int64_t i = 0; i <= 3 * n; ++i) {
    items.push_back(i + 1);
  }

  for (int64_t i = 1; i <= n; ++i) {
    for (int64_t j = 1; j < (2 * n); ++j) {
      int64_t k = i + j + 1;
      /* These are the indices of the slots. They are declared here for clarify.
       * The compiler will fold them forward.
       */
      int64_t s_j = n + j;
      int64_t s_k = n + k;
      std::vector<int64_t> option;
      // Here will just follow the option definition given in Knuth (p. 68).
      option.push_back(i);
      option.push_back(s_j);
      option.push_back(s_k);
      options.push_back(option);
    }
  }
  this->exact_cover_problem = new ExactCoverProblem(items, options);
}

void LangfordPairsProblem::solve() { this->exact_cover_problem->solve(); }
} // namespace algorithm_x
