//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexWeakFormProblemComposer.h"
#include "ComplexEquationSystemProblemOperator.h"

registerMooseObject("MooseApp", MFEMComplexWeakFormProblemComposer);

MFEMComplexWeakFormProblemComposer::MFEMComplexWeakFormProblemComposer(
    const InputParameters & parameters)
  : MFEMProblemComposer(parameters)
{
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
MFEMComplexWeakFormProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  if (mfem_problem.getNumericType() != MFEMProblem::NumericType::COMPLEX)
    mooseError("Wrong numeric type. Please set the Problem numeric type to 'complex'.");

  mfem_problem.getProblemData().eqn_system = std::make_shared<Moose::MFEM::ComplexEquationSystem>();
  return std::make_shared<Moose::MFEM::ComplexEquationSystemProblemOperator>(mfem_problem);
}

#endif
