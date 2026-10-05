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

InputParameters
MFEMComplexWeakForm::validParams()
{
  InputParameters params = MFEMWeakFormBase::validParams();
  params.addClassDescription("Builds a complex-valued MFEM ComplexEquationSystem from the "
                             "complex kernels and boundary conditions named by this weak form.");
  return params;
}

MFEMComplexWeakForm::MFEMComplexWeakForm(const InputParameters & parameters)
  : MFEMWeakFormBase(parameters)
{
}

Moose::MFEM::ComplexEquationSystem &
MFEMComplexWeakForm::complexEquationSystem()
{
  return cast_ref<Moose::MFEM::ComplexEquationSystem &>(*_equation_system);
}

void
MFEMComplexWeakForm::addBoundaryCondition(const std::string & name,
                                          std::shared_ptr<MFEMBoundaryCondition> bc)
{
  if (auto integrated_bc = std::dynamic_pointer_cast<MFEMComplexIntegratedBC>(bc))
    complexEquationSystem().AddComplexIntegratedBC(std::move(integrated_bc));
  else if (auto essential_bc = std::dynamic_pointer_cast<MFEMComplexEssentialBC>(bc))
    complexEquationSystem().AddComplexEssentialBCs(std::move(essential_bc));
  else
    mooseError("Unsupported bc of name '", name, "' detected.");
}

void
MFEMComplexWeakForm::addKernel(const std::string & name, std::shared_ptr<MFEMKernel> kernel)
{
  if (auto complex_kernel = std::dynamic_pointer_cast<MFEMComplexKernel>(kernel))
    complexEquationSystem().AddComplexKernel(std::move(complex_kernel));
  else
    mooseError("Unsupported kernel of name '", name, "' detected.");
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMComplexWeakForm::makeEquationSystem()
{
  return std::make_shared<Moose::MFEM::ComplexEquationSystem>();
}

#endif
