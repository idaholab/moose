//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "CustomProblemOperator.h"

// The custom operator constructor
CustomProblemOperator::CustomProblemOperator(MFEMProblem & mfem_problem)
  : Moose::MFEM::ProblemOperator(mfem_problem), _one(1.0)
{
}

void
CustomProblemOperator::Init(mfem::BlockVector &)
{
  // Get the FE-space and Variable that were just built
  auto fes = _problem.getProblemData().fespaces.Get("H1");
  auto gridfunction = _problem.getGridFunction("u");

  // Boundary conditions
  fes->GetBoundaryTrueDofs(_boundary_dofs);

  // Build the linear form
  _b = new mfem::ParLinearForm(fes);
  _b->AddDomainIntegrator(new mfem::DomainLFIntegrator(_one));
  _b->Assemble();

  // Build the bilinear form
  _a = new mfem::ParBilinearForm(fes);
  _a->AddDomainIntegrator(new mfem::DiffusionIntegrator);
  _a->Assemble();

  // Form the linear system
  _a->FormLinearSystem(_boundary_dofs, *gridfunction, *_b, _problem_operator, _X, _B);
}

void
CustomProblemOperator::Solve()
{
  // Set the operator and solve the equation
  _problem_data.jacobian_solver->SetOperator(*_problem_operator);
  _problem_data.jacobian_solver->GetSolver().Mult(_B, _X);

  // Set the data in the grid function
  auto grid_function = _problem.getGridFunction("u");
  grid_function->SetFromTrueDofs(_X);
}

#endif
