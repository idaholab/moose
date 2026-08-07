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

#include "MFEMProblem.h"
#include "ProblemComposerBase.h"
#include "ProblemOperator.h"
#include "ProblemOperatorBase.h"
#include "CustomDummyProblemOperator.h"

namespace Moose::MFEM
{
/**
 * Custom Dummy Operator builder required to build MFEM Problem Operators
 * used by the executioner
 */
class CustomProblemComposer : public ProblemComposerBase
{
public:
  static InputParameters validParams();

  CustomProblemComposer(const InputParameters & parameters) : ProblemComposerBase(parameters) {};

  ~CustomProblemComposer() = default;

  /// Returns a pointer to a freshly minted operator.
  std::shared_ptr<ProblemOperatorBase> createProblemOperator(MFEMProblem & _mfem_problem) override;
};
};

#endif
