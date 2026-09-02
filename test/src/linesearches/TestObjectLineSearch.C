//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestObjectLineSearch.h"

#include "NonlinearSystemBase.h"
#include "libmesh/petsc_nonlinear_solver.h"

registerMooseObject("MooseTestApp", TestObjectLineSearch);

InputParameters
TestObjectLineSearch::validParams()
{
  InputParameters params = LineSearch::validParams();
  params.addClassDescription(
      "A minimal LineSearch that accepts the full Newton step, used to regression-test that "
      "each nonlinear system's LineSearch object is dispatched independently by PETSc.");
  return params;
}

TestObjectLineSearch::TestObjectLineSearch(const InputParameters & parameters)
  : LineSearch(parameters)
{
}

void
TestObjectLineSearch::lineSearch()
{
  auto * solver = dynamic_cast<PetscNonlinearSolver<Real> *>(_nl.nonlinearSolver());
  if (!solver)
    mooseError(
        "This line search operates only with Petsc, so Petsc must be your nonlinear solver.");
  SNES snes = solver->snes();

  Vec X, F, Y, W;
  SNESLineSearch line_search;
  LibmeshPetscCall(SNESGetLineSearch(snes, &line_search));
  LibmeshPetscCall(SNESLineSearchGetVecs(line_search, &X, &F, &Y, &W, nullptr));
  LibmeshPetscCall(SNESLineSearchSetReason(line_search, SNES_LINESEARCH_SUCCEEDED));

  // Accept the full Newton step. This line search's only purpose is to confirm that PETSc
  // dispatches to the correct per-nonlinear-system LineSearch object, so no actual searching
  // is performed.
  LibmeshPetscCall(VecWAXPY(W, -1., Y, X));
  LibmeshPetscCall(SNESComputeFunction(snes, W, F));
  LibmeshPetscCall(VecCopy(W, X));
  LibmeshPetscCall(SNESLineSearchComputeNorms(line_search));

  _console << "TestObjectLineSearch '" << name() << "' invoked for nl_sys_num " << _nl_sys_num
           << std::endl;
}
