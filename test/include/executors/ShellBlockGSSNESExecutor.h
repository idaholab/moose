//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SNESNPCExecutor.h"
#include "MooseEnum.h"

class NewtonSNESExecutor;

/**
 * Test executor that wraps a SNESSHELL to perform a block Gauss-Seidel sweep over
 * multiple nonlinear systems.  Used to exercise the multi-system Case 3 path of
 * NewtonSNESExecutor while being honest that no named PETSc SNES type implements
 * this algorithm.
 *
 * sweep_type = multiplicative           forward sweep only (1..N)
 * sweep_type = symmetric_multiplicative forward then backward sweep (1..N, N-1..1)
 */
class ShellBlockGSSNESExecutor : public SNESNPCExecutor
{
public:
  static InputParameters validParams();
  ShellBlockGSSNESExecutor(const InputParameters & params);
  virtual ~ShellBlockGSSNESExecutor();

  virtual Result run() override;
  virtual PetscErrorCode applyBA(Mat A, Vec X, Vec Y) override;

protected:
  virtual void setupSNES() override;

private:
  std::vector<NewtonSNESExecutor *> _sub_snes;
  const MooseEnum _sweep_type;
  Vec _block_residual = nullptr;
  Vec _block_update = nullptr;

  static PetscErrorCode shellSolveCallback(SNES snes, Vec x);
  PetscErrorCode applyBlockUpdate(Mat A, Vec rhs, Vec Y, PetscInt i);
};
