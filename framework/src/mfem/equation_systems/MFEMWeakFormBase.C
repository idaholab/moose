//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWeakFormBase.h"
#include "TimeDependentEquationSystem.h"
#include "EigenproblemEquationSystem.h"
#include "ComplexEquationSystem.h"
#include "MFEMEigenproblem.h"

InputParameters
MFEMWeakFormBase::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("MFEMWeakFormBase");
  params.registerSystemAttributeName("MFEMWeakFormBase");
  params.addParam<std::vector<MFEMBoundaryConditionName>>(
      "bcs", {}, "List of boundary conditions to add to the weak form");
  params.addParam<std::vector<MFEMKernelName>>(
      "kernels", {}, "List of kernels to add to the weak form");
  return params;
}

MFEMWeakFormBase::MFEMWeakFormBase(const InputParameters & parameters)
  : MFEMObject(parameters),
    _bc_names(getParam<std::vector<MFEMBoundaryConditionName>>("bcs")),
    _kernel_names(getParam<std::vector<MFEMKernelName>>("kernels"))
{
}

void
MFEMWeakFormBase::initEquationSystem(std::shared_ptr<Moose::MFEM::EquationSystem> equation_system)
{
  auto & problem_data = getMFEMProblem().getProblemData();
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
    equation_system->SetGradientRequired(problem_data.nonlinear_solver->RequiresGradient());

  equation_system->SetCoefficientManager(problem_data.coefficients);

  // Set up initial conditions
  equation_system->Init(problem_data.gridfunctions,
                        problem_data.cmplx_gridfunctions,
                        getMFEMProblem()._default_assembly_level);
}

#endif
