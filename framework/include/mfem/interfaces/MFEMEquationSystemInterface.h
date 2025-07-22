//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "MFEMProblemData.h"

namespace Moose::MFEM
{
/**
 * Interface required for:
 * - EquationSystemProblemOperator
 * - TimeDomainEquationSystemProblemOperator
 */
class MFEMEquationSystemInterface
{
public:
  MFEMEquationSystemInterface() = default;
  virtual ~MFEMEquationSystemInterface() = default;

  /// Returns a pointer to the operator's equation system.
  [[nodiscard]] virtual Moose::MFEM::EquationSystem * GetEquationSystem() const = 0;
};
}

#endif
