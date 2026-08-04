//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "CustomDummyProblemOperator.h"

// The custom operator constructor
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob0)
  : Moose::MFEM::ProblemOperator(prob0), _one(1.000)
{
  // Retrieve the FE-space and gridFunction
  const std::string _fe_space_name = "h1";
  const std::string _grid_function_name = "var0";
  auto _fes = prob0.getProblemData().fespaces.GetShared(_fe_space_name);
  auto _grid_function = prob0.getProblemData().gridfunctions.GetShared(_grid_function_name);

  // Boundary conditions
  *_grid_function = 0.00;
  _fes->GetBoundaryTrueDofs(_boundary_dofs);

  // Build the linear form
  _b = new mfem::ParLinearForm(&(*_fes));
  _b->AddDomainIntegrator(new mfem::DomainLFIntegrator(_one));
  _b->Assemble();

  // Build the bilinear form
  _a = new mfem::ParBilinearForm(&(*_fes));
  _a->AddDomainIntegrator(new mfem::DiffusionIntegrator);
  _a->Assemble();

  // Form the linear system
  _a->FormLinearSystem(_boundary_dofs, *_grid_function, *_b, _problem_operator, _X, _B);
};

void
CustomDummyProblemOperator::Solve()
{
  // Set the operator and solve the equation
  _problem_data.jacobian_solver->SetOperator(*_problem_operator);
  _problem_data.jacobian_solver->GetSolver().Mult(_B, _X);

  // Set the data in the grid function
  const std::string _grid_function_name = "var0";
  auto _grid_function = _problem_data.gridfunctions.GetShared(_grid_function_name);
  _grid_function->SetFromTrueDofs(_X);
};

void
CustomDummyProblemOperator::Mult(const mfem::Vector & x, mfem::Vector & y) const
{
  _problem_operator->Mult(x, y);
}

#endif
