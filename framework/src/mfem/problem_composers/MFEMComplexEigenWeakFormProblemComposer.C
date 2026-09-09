//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexEigenWeakFormProblemComposer.h"
#include "ComplexEigenproblemESProblemOperator.h"

registerMooseObject("MooseApp", MFEMComplexEigenWeakFormProblemComposer);

MFEMComplexEigenWeakFormProblemComposer::MFEMComplexEigenWeakFormProblemComposer(
    const InputParameters & parameters)
  : MFEMProblemComposer(parameters)
{
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
MFEMComplexEigenWeakFormProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  auto * eigen_problem = dynamic_cast<MFEMEigenproblem *>(&mfem_problem);
  if (!eigen_problem)
    mooseError("Not an eigenvalue problem. ");

  if (mfem_problem.getNumericType() != MFEMProblem::NumericType::COMPLEX)
    mooseError("Wrong numeric type. Please set the Problem numeric type to 'complex'.");

  mfem_problem.getProblemData().eqn_system =
      std::make_shared<Moose::MFEM::ComplexEigenproblemEquationSystem>(*eigen_problem);
  return std::make_shared<Moose::MFEM::ComplexEigenproblemESProblemOperator>(mfem_problem);
}

#endif
