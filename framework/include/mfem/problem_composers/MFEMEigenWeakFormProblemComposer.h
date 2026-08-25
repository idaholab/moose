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
#include "MFEMProblemComposer.h"

/**
 * MFEMEigenWeakFormProblemComposer required to build an
 * instance of EigenproblemESProblemOperator
 */
class MFEMEigenWeakFormProblemComposer : public MFEMProblemComposer
{
public:
  MFEMEigenWeakFormProblemComposer(const InputParameters & parameters)
    : MFEMProblemComposer(parameters)
  {
  }

  ~MFEMEigenWeakFormProblemComposer() = default;

  /// Returns a pointer to a freshly minted problem operator.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & mfem_problem) override;
};

#endif
