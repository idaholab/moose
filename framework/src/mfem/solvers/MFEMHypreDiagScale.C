//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreDiagScale.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreDiagScale);

InputParameters
MFEMHypreDiagScale::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("Hypre diagonal scaling (Jacobi) preconditioner for MFEM equation "
                             "systems, which can precondition hypre Krylov solvers.");
  return params;
}

MFEMHypreDiagScale::MFEMHypreDiagScale(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters)
{
  ConstructSolver();
}

void
MFEMHypreDiagScale::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreDiagScale>();
  solver->iterative_mode = getParam<bool>("use_initial_guess");
  _solver = std::move(solver);
}

#endif
