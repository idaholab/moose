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
#include "MFEMIntegratedBC.h"
#include "MFEMEssentialBC.h"
#include "MFEMEssentialConstraint.h"
#include "MFEMComplexEssentialConstraint.h"

registerMooseObject("MooseApp", MFEMWeakForm);

InputParameters
MFEMWeakForm::validParams()
{
  InputParameters params = MFEMWeakFormBase::validParams();
  params.addClassDescription("Builds a real-valued MFEM EquationSystem from the kernels and "
                             "boundary conditions named by this weak form.");
  return params;
}

MFEMWeakForm::MFEMWeakForm(const InputParameters & parameters) : MFEMWeakFormBase(parameters) {}

void
MFEMWeakForm::addBoundaryCondition(const std::string & name,
                                   std::shared_ptr<MFEMBoundaryCondition> bc)
{
  if (auto integrated_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc))
    _equation_system->AddIntegratedBC(std::move(integrated_bc));
  else if (auto essential_bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc))
    _equation_system->AddEssentialBC(std::move(essential_bc));
  else
    mooseError("Unsupported bc of name '", name, "' detected.");
}

void
MFEMWeakForm::addKernel(const std::string & /*name*/, std::shared_ptr<MFEMKernel> kernel)
{
  _equation_system->AddKernel(std::move(kernel));
}

void
MFEMWeakForm::addConstraint(const std::string & name, std::shared_ptr<MFEMConstraint> constraint)
{
  if (auto essential_constraint = std::dynamic_pointer_cast<MFEMEssentialConstraint>(constraint))
    _equation_system->AddEssentialConstraint(std::move(essential_constraint));
  else if (std::dynamic_pointer_cast<MFEMComplexEssentialConstraint>(constraint))
    mooseError("Cannot add constraint with name '",
               name,
               "' because there is no corresponding complex equation system.");
  else
    mooseError("Unsupported constraint of name '", name, "' detected.");
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMWeakForm::makeEquationSystem()
{
  return std::make_shared<Moose::MFEM::EquationSystem>();
}

#endif
