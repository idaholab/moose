//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTimeDependentWeakForm.h"
#include "TimeDependentEquationSystem.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMTimeDependentWeakForm);

InputParameters
MFEMTimeDependentWeakForm::validParams()
{
  InputParameters params = MFEMWeakForm::validParams();
  params.addClassDescription("Builds a real-valued MFEM TimeDependentEquationSystem from the "
                             "kernels and boundary conditions named by this weak form.");
  return params;
}

MFEMTimeDependentWeakForm::MFEMTimeDependentWeakForm(const InputParameters & parameters)
  : MFEMWeakForm(parameters)
{
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMTimeDependentWeakForm::makeEquationSystem()
{
  return std::make_shared<Moose::MFEM::TimeDependentEquationSystem>(
      getMFEMProblem().getProblemData().time_derivative_map);
}

#endif
