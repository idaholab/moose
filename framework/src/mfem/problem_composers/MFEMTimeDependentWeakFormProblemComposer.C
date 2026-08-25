//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTimeDependentWeakFormProblemComposer.h"
#include "TimeDependentEquationSystemProblemOperator.h"

registerMooseObject("MooseApp", MFEMTimeDependentWeakFormProblemComposer);

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
MFEMTimeDependentWeakFormProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  mfem_problem.getProblemData().eqn_system =
      std::make_shared<Moose::MFEM::TimeDependentEquationSystem>(
          mfem_problem.getProblemData().time_derivative_map);
  return std::make_shared<Moose::MFEM::TimeDependentEquationSystemProblemOperator>(mfem_problem);
}

#endif
