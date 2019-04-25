//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExternalProblem.h"
#include "PETScDiffusionFDM.h"
#include "ExternalPetscSolverApp.h"

class ExternalPETScProblem;

template <>
InputParameters validParams<ExternalPETScProblem>();

/**
 * This is an interface to call a pure PETSc solver.
 * We also sync the PETSc solution to moose variables,
 * and then these variables can be coupled to other
 * moose applications
 */
class ExternalPETScProblem : public ExternalProblem
{
public:
  ExternalPETScProblem(const InputParameters & params);
#if LIBMESH_HAVE_PETSC
  ~ExternalPETScProblem() { VecDestroy(&_petsc_sol); }
#endif

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /*direction*/) override;

  virtual bool converged() override { return true; }

private:
  /// The name of the variable to transfer to
  const VariableName & _sync_to_var_name;
  ExternalPetscSolverApp & _petsc_app;

#if LIBMESH_HAVE_PETSC
  /// PETSc solver
  TS & _ts;
  /// PETSc solver solution
  Vec _petsc_sol;
#endif
};

