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

/**
 * This is an interface to call a pure PETSc solver.
 * We also sync the PETSc solution to moose variables,
 * and then these variables can be coupled to other
 * moose applications
 */
class ExternalPETScProblem : public ExternalProblem
{
public:
  static InputParameters validParams();

  ExternalPETScProblem(const InputParameters & params);

  ~ExternalPETScProblem();

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /*direction*/) override;

  virtual bool converged() override { return _petsc_converged; }

  virtual void advanceState() override;

  virtual Real computeResidualL2Norm() override;

  Vec & solutionOld() { return _petsc_sol_old; }

  Vec & currentSolution() { return _petsc_sol; }

  Vec & udot() { return _petsc_udot; }

  TS & getPetscTS() { return _ts; }

private:
  /// The name of the variable to transfer to
  const VariableName & _sync_to_var_name;
  /// If PETSc solver converged
  PetscBool _petsc_converged;
  ExternalPetscSolverApp & _external_petsc_app;
  /// PETSc solver
  TS & _ts;
  /// PETSc solver solution
  Vec & _petsc_sol;
  /// Solution at the previous time step
  Vec & _petsc_sol_old;
  /// Udot (u_n-u_{n-1})/dt
  Vec & _petsc_udot;
  /// RHS vector
  Vec _petsc_rhs;
};
