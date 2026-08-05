//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWeakForm.h"
#include "TimeDependentEquationSystem.h"
#include "EigenproblemEquationSystem.h"
#include "ComplexEquationSystem.h"
#include "MFEMEigenproblem.h"

registerMooseObject("MooseApp", MFEMWeakForm);

MFEMWeakForm::MFEMWeakForm(const InputParameters & parameters) : MFEMWeakFormBase(parameters) {}

void
MFEMWeakForm::addBoundaryCondition(const std::string & name,
                                   std::shared_ptr<MFEMBoundaryCondition> bc)
{
  const auto & mfem_bc = *bc;
  if (dynamic_cast<const MFEMIntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc);
    _equation_system->AddIntegratedBC(std::move(integrated_bc));
  }
  else if (dynamic_cast<const MFEMEssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc);
    _equation_system->AddEssentialBC(std::move(essential_bc));
  }
  else
    mooseError("Unsupported bc of name '", name, "' detected.");
}

void
MFEMWeakForm::addKernel(const std::string & /*name*/, std::shared_ptr<MFEMKernel> kernel)
{
  _equation_system->AddKernel(std::move(kernel));
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMWeakForm::createEquationSystem()
{
  _equation_system = std::make_shared<Moose::MFEM::EquationSystem>();
  initEquationSystem(_equation_system);
  return _equation_system;
}

#endif
