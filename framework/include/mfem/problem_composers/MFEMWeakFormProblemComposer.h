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

/**
 * MFEMWeakFormProblemComposer required to build an instance of
 * EquationSystemProblemOperator
 */
class MFEMWeakFormProblemComposer : public MFEMProblemComposer
{
public:
  MFEMWeakFormProblemComposer(const InputParameters & parameters);

  /// Returns a pointer to a freshly minted problem operator.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & mfem_problem) override;
};

#endif
