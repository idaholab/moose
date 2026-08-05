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
#include "TimeDependentEquationSystem.h"

registerMooseObject("MooseApp", MFEMEigenproblemWeakForm);

MFEMEigenproblemWeakForm::MFEMEigenproblemWeakForm(const InputParameters & parameters)
  : MFEMWeakForm(parameters)
{
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMEigenproblemWeakForm::createEquationSystem()
{
  _equation_system = std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();
  initEquationSystem(_equation_system);
  return _equation_system;
}

#endif
