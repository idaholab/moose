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

#include "MFEMEigenproblem.h"
#include "ProblemOperatorBase.h"
#include "ProblemComposerBase.h"

class MFEMEigenproblem;

namespace Moose::MFEM
{
/**
 * Steady builder required to build MFEM Problem Operators
 * used by the steady executioner
 */
class SteadyProblemComposer : public ProblemComposerBase
{
public:
  static InputParameters validParams();

  SteadyProblemComposer(const InputParameters & parameters);

  ~SteadyProblemComposer() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<ProblemOperatorBase> createProblemOperator(MFEMProblem & mfem_problem) override;
};
}

#endif
