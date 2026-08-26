//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "CustomDummyProblemOperator.h"

// The custom operator constructor
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob_ex0p)
  : Moose::MFEM::ProblemOperator(prob_ex0p), _one(1.000)
{
}

void
CustomDummyProblemOperator::Init(mfem::BlockVector &)
{
  // Get the FE-space and Variable that were just built
  auto fes = _problem.getProblemData().fespaces.GetShared("prob_ex0p_h1");
  auto grid_function = _problem.getGridFunction("prob_ex0p_var0");

  // Boundary conditions
  fes->GetBoundaryTrueDofs(_boundary_dofs);

  // Build the linear form
  _b = new mfem::ParLinearForm(&(*fes));
  _b->AddDomainIntegrator(new mfem::DomainLFIntegrator(_one));
  _b->Assemble();

  // Build the bilinear form
  _a = new mfem::ParBilinearForm(&(*fes));
  _a->AddDomainIntegrator(new mfem::DiffusionIntegrator);
  _a->Assemble();

  // Form the linear system
  _a->FormLinearSystem(_boundary_dofs, *grid_function, *_b, _problem_operator, _X, _B);
}

void
CustomDummyProblemOperator::Solve()
{
  // Set the operator and solve the equation
  _problem_data.jacobian_solver->SetOperator(*_problem_operator);
  _problem_data.jacobian_solver->GetSolver().Mult(_B, _X);

  // Set the data in the grid function
  auto grid_function = _problem.getGridFunction("prob_ex0p_var0");
  grid_function->SetFromTrueDofs(_X);
}

void
CustomDummyProblemOperator::Mult(const mfem::Vector & x, mfem::Vector & y) const
{
  _problem_operator->Mult(x, y);
}

#endif
