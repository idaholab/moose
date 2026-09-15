//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWeakFormProblemComposer.h"
#include "EquationSystemProblemOperator.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMWeakFormProblemComposer);

MFEMWeakFormProblemComposer::MFEMWeakFormProblemComposer(const InputParameters & parameters)
  : MFEMWeakFormProblemComposerBase(parameters)
{
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
MFEMWeakFormProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  if (mfem_problem.getNumericType() != MFEMProblem::NumericType::REAL)
    mooseError("Wrong numeric type. Please set the Problem numeric type to 'real'.");

  return std::make_shared<Moose::MFEM::EquationSystemProblemOperator>(mfem_problem,
                                                                      _weak_form_name);
}

#endif
