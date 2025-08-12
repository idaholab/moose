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

namespace Moose::MFEM
{
/**
 * Interface required for MFEM executioners to add and get ProblemOperators:
 * - MFEMTransient
 * - MFEMSteady
 */
class ProblemOperatorInterface
{
public:
  ProblemOperatorInterface() = default;
  virtual ~ProblemOperatorInterface() = default;

  /// Returns a pointer to the operator's equation system.
  virtual std::vector<std::shared_ptr<ProblemOperatorBase>> & getProblemOperators()
  {
    return _problem_operators;
  }

  /// Add an MFEM problem operator. Takes ownership.
  virtual void addProblemOperator(std::shared_ptr<ProblemOperatorBase> problem_operator)
  {
    _problem_operators.push_back(std::move(problem_operator));
  }

private:
  std::vector<std::shared_ptr<ProblemOperatorBase>> _problem_operators;
};
}

#endif
