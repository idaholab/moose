#include "THMTestUtils.h"

Real
computeFDPerturbation(const Real & value, const Real & relative_perturbation)
{
  return std::max(relative_perturbation * std::abs(value), PERTURBATION_MIN);
}
