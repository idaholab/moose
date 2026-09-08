//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexWeakForm.h"
#include "MFEMComplexIntegratedBC.h"
#include "MFEMComplexEssentialBC.h"
#include "MFEMComplexKernel.h"

registerMooseObject("MooseApp", MFEMComplexWeakForm);

MFEMComplexWeakForm::MFEMComplexWeakForm(const InputParameters & parameters)
  : MFEMWeakFormBase(parameters)
{
}

Moose::MFEM::ComplexEquationSystem &
MFEMComplexWeakForm::complexEquationSystem()
{
  mooseAssert(std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(_equation_system),
              "The equation system built by MFEMComplexWeakForm is not a ComplexEquationSystem.");
  return static_cast<Moose::MFEM::ComplexEquationSystem &>(*_equation_system);
}

void
MFEMComplexWeakForm::addBoundaryCondition(const std::string & name,
                                          std::shared_ptr<MFEMBoundaryCondition> bc)
{
  const auto & mfem_bc = *bc;
  if (dynamic_cast<const MFEMComplexIntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<MFEMComplexIntegratedBC>(bc);
    complexEquationSystem().AddComplexIntegratedBC(std::move(integrated_bc));
  }
  else if (dynamic_cast<const MFEMComplexEssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<MFEMComplexEssentialBC>(bc);
    complexEquationSystem().AddComplexEssentialBCs(std::move(essential_bc));
  }
  else
    mooseError("Unsupported bc of name '", name, "' detected.");
}

void
MFEMComplexWeakForm::addKernel(const std::string & name, std::shared_ptr<MFEMKernel> kernel)
{
  auto complex_kernel = std::dynamic_pointer_cast<MFEMComplexKernel>(kernel);
  if (!complex_kernel)
    mooseError("Unsupported kernel of name '", name, "' detected.");
  complexEquationSystem().AddComplexKernel(std::move(complex_kernel));
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMComplexWeakForm::makeEquationSystem()
{
  return std::make_shared<Moose::MFEM::ComplexEquationSystem>();
}

#endif
