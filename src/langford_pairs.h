#ifndef LANGFORD_PAIRS_H
#define LANGFORD_PAIRS_H

#include "algorithm_x.h"
#include "xc_equivalent_problem.h"
#include <cstdint>
#include <string>
#include <vector>

namespace algorithm_x {
class LangfordPairsProblem : public XcEquivalentProblem {
public:
  LangfordPairsProblem() = delete;
  LangfordPairsProblem(int64_t n);
  LangfordPairsProblem(LangfordPairsProblem &other) = delete;
  LangfordPairsProblem(LangfordPairsProblem &&other) = delete;
  LangfordPairsProblem &operator=(LangfordPairsProblem &other) = delete;
  LangfordPairsProblem &operator=(LangfordPairsProblem &&other) = delete;
  ~LangfordPairsProblem();

  void solve(bool find_all_solutions=true);
  const std::string solutions_string();

private:
  int64_t n;
  std::vector<int64_t> items;
  std::vector<std::vector<int64_t>> options;
  void initialize();
};
} // namespace algorithm_x

#endif // #define LANGFORD_PAIRS_H
