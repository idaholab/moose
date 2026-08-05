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

MFEMWeakForm::MFEMWeakForm(const InputParameters & parameters) : MFEMWeakFormBase(parameters)
{
  auto & problem_data = getMFEMProblem().getProblemData();
  if (getMFEMProblem().isTransient())
  {
    _equation_system = std::make_shared<Moose::MFEM::TimeDependentEquationSystem>(
        problem_data.time_derivative_map);
  }
  else
  {
    if (getMFEMProblem().getNumericType() == MFEMProblem::NumericType::REAL)
    {
      if (dynamic_cast<MFEMEigenproblem *>(&getMFEMProblem()))
        _equation_system = std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();
      else
        _equation_system = std::make_shared<Moose::MFEM::EquationSystem>();
    }
    else if (getMFEMProblem().getNumericType() == MFEMProblem::NumericType::COMPLEX)
    {
      _equation_system = std::make_shared<Moose::MFEM::ComplexEquationSystem>();
    }
    else
      mooseError("Unknown numeric type. "
                 "Please set the Problem numeric type to either 'real' or 'complex'.");
  }

  if (_bc_names.empty()) // default to all BCs added by user
    for (auto & [bc_name, bc] : problem_data.bcs)
      addBoundaryCondition(bc_name, bc);
  else
    for (const auto & bc_name : _bc_names)
      addBoundaryCondition(bc_name, problem_data.bcs.GetShared(bc_name));

  if (_kernel_names.empty()) // default to all kernels added by user
    for (auto & [kernel_name, kernel] : problem_data.kernels)
      addKernel(kernel_name, kernel);
  else
    for (const auto & kernel_name : _kernel_names)
      addKernel(kernel_name, problem_data.kernels.GetShared(kernel_name));

  if (problem_data.nonlinear_solver)
    _equation_system->SetGradientRequired(problem_data.nonlinear_solver->RequiresGradient());

  _equation_system->SetCoefficientManager(problem_data.coefficients);

  // Set up initial conditions
  _equation_system->Init(problem_data.gridfunctions,
                         problem_data.cmplx_gridfunctions,
                         getMFEMProblem()._default_assembly_level);
}

void
MFEMWeakForm::addBoundaryCondition(const std::string & name,
                                   std::shared_ptr<MFEMBoundaryCondition> bc)
{
  const auto & mfem_bc = *bc;

  if (dynamic_cast<const MFEMIntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<Moose::MFEM::EquationSystem>(_equation_system);
    if (eqsys)
      eqsys->AddIntegratedBC(std::move(integrated_bc));
    else
      mooseError("Cannot add integrated BC with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const MFEMComplexIntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<MFEMComplexIntegratedBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(_equation_system);
    if (eqsys)
      eqsys->AddComplexIntegratedBC(std::move(integrated_bc));
    else
      mooseError("Cannot add complex integrated BC with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const MFEMComplexEssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<MFEMComplexEssentialBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(_equation_system);
    if (eqsys)
      eqsys->AddComplexEssentialBCs(std::move(essential_bc));
    else
      mooseError("Cannot add boundary condition with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const MFEMEssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<Moose::MFEM::EquationSystem>(_equation_system);
    if (eqsys)
      eqsys->AddEssentialBC(std::move(essential_bc));
    else
      mooseError("Cannot add boundary condition with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else
  {
    mooseError("Unsupported bc of name '", name, "' detected.");
  }
}

void
MFEMWeakForm::addKernel(const std::string & name, std::shared_ptr<MFEMKernel> kernel)
{
  const auto & kernel_object = *kernel;

  if (dynamic_cast<const MFEMComplexKernel *>(&kernel_object))
  {
    auto complex_kernel = std::dynamic_pointer_cast<MFEMComplexKernel>(kernel);
    auto eqsys = std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(_equation_system);
    if (eqsys)
      eqsys->AddComplexKernel(std::move(complex_kernel));
    else
      mooseError("Cannot add complex kernel with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else
  {
    auto eqsys = std::dynamic_pointer_cast<Moose::MFEM::EquationSystem>(_equation_system);
    if (eqsys)
      eqsys->AddKernel(std::move(kernel));
    else
      mooseError("Cannot add kernel with name '" + name +
                 "' because there is no corresponding equation system.");
  }
}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMWeakForm::createEquationSystem()
{
  return _equation_system;
}

#endif
