//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SNESExecutor.h"

/**
 * Test executor that wraps a SNESSHELL to perform a block Gauss-Seidel sweep over
 * multiple nonlinear systems.  Used to exercise the multi-system Case 3 path of
 * NewtonSNESExecutor while being honest that no named PETSc SNES type implements
 * this algorithm.
 */
class ShellBlockGSSNESExecutor : public SNESExecutor
{
public:
  static InputParameters validParams();
  ShellBlockGSSNESExecutor(const InputParameters & params);

  virtual Result run() override;
  virtual void initialSetup() override;

protected:
  virtual void setupSNES() override;

private:
  std::vector<SNESExecutor *> _sub_snes;

  static PetscErrorCode shellSolveCallback(SNES snes, Vec x);
};
