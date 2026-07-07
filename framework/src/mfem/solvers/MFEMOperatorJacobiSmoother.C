//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMOperatorJacobiSmoother.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMOperatorJacobiSmoother);

InputParameters
MFEMOperatorJacobiSmoother::validParams()
{
  InputParameters params = Moose::MFEM::LORLinearSolverBase<mfem::OperatorJacobiSmoother>::validParams();
  params.addClassDescription("MFEM solver for performing Jacobi smoothing of the equation system.");
  params.addParam<double>(
      "damping",
      1.0,
      "Damping factor omega for the scaled-Jacobi iteration y = omega * D^{-1} * x. "
      "When used as a multigrid smoother, omega must satisfy omega < 2/lambda_max(D^{-1}A).");
  return params;
}

MFEMOperatorJacobiSmoother::MFEMOperatorJacobiSmoother(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::OperatorJacobiSmoother>(parameters)
{
  ConstructSolver();
}

void
MFEMOperatorJacobiSmoother::SetSolverParameters(mfem::OperatorJacobiSmoother & solver)
{
  solver.iterative_mode = getParam<bool>("use_initial_guess");
}

void
MFEMOperatorJacobiSmoother::ConstructSolver()
{
  auto solver = std::make_unique<mfem::OperatorJacobiSmoother>(getParam<double>("damping"));
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMOperatorJacobiSmoother::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (_lor)
    SetLORSolver(*this, *_equation_system);
}

#endif
