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

#include "ProblemOperatorBase.h"
#include "ProblemOperatorBuilderBase.h"

namespace Moose::MFEM
{
/**
 * Transient builder required to build MFEM Problem Operators
 * used by the executioner
 */
class ProblemOperatorBuilderTransient : public ProblemOperatorBuilderBase
{
public:
  static InputParameters validParams();

  ProblemOperatorBuilderTransient(const InputParameters & parameters);

  ~ProblemOperatorBuilderTransient() = default;

  /// Returns a pointer to a problem operator.
  std::shared_ptr<ProblemOperatorBase> createProblemOperator(MFEMProblem & mfemProb) override;
};
}

#endif
