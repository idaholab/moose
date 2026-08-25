//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMEigenWeakFormProblemComposer.h"
#include "EigenproblemESProblemOperator.h"

namespace Moose::MFEM
{
registerMooseObject("MooseApp", MFEMEigenWeakFormProblemComposer);
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
Moose::MFEM::MFEMEigenWeakFormProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  if (!dynamic_cast<MFEMEigenproblem *>(&mfem_problem))
    mooseError("Not an eigenvalue problem. ");

  mfem_problem.getProblemData().eqn_system =
      std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();
  return std::make_shared<Moose::MFEM::EigenproblemESProblemOperator>(mfem_problem);
}

#endif
