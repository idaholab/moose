//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMEigenproblemWeakForm.h"
#include "EigenproblemEquationSystem.h"

registerMooseObject("MooseApp", MFEMEigenproblemWeakForm);

InputParameters
MFEMEigenproblemWeakForm::validParams()
{
  InputParameters params = MFEMWeakForm::validParams();
  params.addClassDescription("Builds a real-valued MFEM EigenproblemEquationSystem from the "
                             "kernels and boundary conditions named by this weak form.");
  return params;
}

MFEMEigenproblemWeakForm::MFEMEigenproblemWeakForm(const InputParameters & parameters)
  : MFEMWeakForm(parameters)
{
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMEigenproblemWeakForm::makeEquationSystem()
{
  return std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();
}

#endif
