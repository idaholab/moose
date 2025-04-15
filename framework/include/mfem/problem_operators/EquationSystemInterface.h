#ifdef MFEM_ENABLED

#pragma once
#include "MFEMProblemData.h"

namespace Moose::MFEM
{
/**
 * Interface required for:
 * - EquationSystemProblemOperator
 * - TimeDomainEquationSystemProblemOperator
 */
class EquationSystemInterface
{
public:
  EquationSystemInterface() = default;
  virtual ~EquationSystemInterface() = default;

  /// Returns a pointer to the operator's equation system.
  [[nodiscard]] virtual Moose::MFEM::EquationSystem * GetEquationSystem() const = 0;
};
}

#endif
