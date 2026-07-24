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

namespace Moose::MFEM
{
/**
 * Transient builder required to build MFEM Problem Operators
 * used by the Transient executioner
 */
class TransientProblemComposer : public ProblemComposerBase
{
public:
  static InputParameters validParams();

  TransientProblemComposer(const InputParameters & parameters);

  ~TransientProblemComposer() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<ProblemOperatorBase> createProblemOperator(MFEMProblem & mfem_problem) override;
};
}

#endif
