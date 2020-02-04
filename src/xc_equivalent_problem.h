#ifndef XC_EQUIVALENT_H
#define XC_EQUIVALENT_H

#include "algorithm_x.h"

namespace algorithm_x {

/**
 * XcEquivalentProblem is a virtaul class defining an interface that a problem
 * must uphold to embed into the exact cover problem. This is useful when a
 * polynomial-time conversation exists to the exact cover problem (as is the
 * case for any NP-complete problem).
 */
class XcEquivalentProblem {
public:
  /* Solve should invole exact_cover_problem.solve() and use
   * exact_cover_problem's solution to generate a solution to the underlying
   * problem. */
  virtual void solve() {}

  virtual const ExactCoverProblem &get_exact_cover_problem() {
    return *exact_cover_problem;
  }

protected:
  ExactCoverProblem *exact_cover_problem;
};

} // namespace algorithm_x

#endif // #define XC_EQUIVALENT_H
