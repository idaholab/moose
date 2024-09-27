#pragma once
#include "problem_builder_base.h"

namespace platypus
{
/**
 * Interface required for:
 * - SteadyStateEquationSystemProblemData
 * - TimeDomainEquationSystemProblemData
 * - EquationSystemProblemOperator
 * - TimeDomainEquationSystemProblemOperator
 */
class EquationSystemInterface
{
public:
  EquationSystemInterface() = default;
  virtual ~EquationSystemInterface() = default;

  /// Returns a pointer to the operator's equation system.
  [[nodiscard]] virtual platypus::EquationSystem * GetEquationSystem() const = 0;
};
}