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

#include "MFEMProblemComposer.h"
#include "CustomDummyProblemOperator.h"

/**
 * Custom Dummy Operator builder required to build MFEM Problem Operators
 * used by the executioner
 */
class CustomProblemComposer : public MFEMProblemComposer
{
public:
  static InputParameters validParams();

  CustomProblemComposer(const InputParameters & parameters) : MFEMProblemComposer(parameters) {};

  ~CustomProblemComposer() = default;

  /// Returns a pointer to a freshly minted operator.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & _mfem_problem) override;
};

#endif
