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

#include "MFEMNonlinearSolverBase.h"

#include <string>

#ifdef MFEM_USE_PETSC

/**
 * MooseObject wrapper for mfem::PetscNonlinearSolver-backed nonlinear solves.
 */
class MFEMPetscNonlinearSolver : public Moose::MFEM::NonlinearSolverBase
{
public:
  static InputParameters validParams();

  MFEMPetscNonlinearSolver(const InputParameters & parameters);

  void constructSolver(const InputParameters & parameters) override;

  void SetOperator(const mfem::Operator & op) override;
  void SetLinearSolver(mfem::Solver & solver) override;
  void Mult(const mfem::Vector & rhs, mfem::Vector & x) override;
  bool requiresGradient() const override { return true; }
  bool usesExternalLinearSolver() const override { return false; }
};

#endif

#endif
