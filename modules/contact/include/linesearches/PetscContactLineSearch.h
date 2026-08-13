//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_macro.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include <petscsnes.h>

#include "ContactLineSearchBase.h"

using namespace libMesh;

/**
 *  Petsc implementation of the contact line search (based on the Petsc LineSearchShell)
 */
class PetscContactLineSearch : public ContactLineSearchBase
{
public:
  static InputParameters validParams();

  PetscContactLineSearch(const InputParameters & parameters);
  ~PetscContactLineSearch();

  virtual void lineSearch() override;

protected:
  PetscNonlinearSolver<Real> * _solver;

  /**
   * Create and configure the secondary backing SNESLineSearch (Component D of
   * CONSTRAINT_SET_STRATEGY_PLAN.md) on first use. It is a standalone SNESLineSearch object (not
   * another shell) set to the user-selected type with its own "contact_backing_" PETSc options
   * prefix, so users can tune it with native PETSc options. Not yet consulted by lineSearch(); the
   * legacy lambda-cut algorithm below remains authoritative until the wrapper is replaced in a
   * later phase.
   */
  void setupBackingLineSearch();

  /// The secondary backing PETSc line search; null until setupBackingLineSearch() is first called
  SNESLineSearch _backing_petsc_line_search;
};
