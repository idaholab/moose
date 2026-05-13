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
 * Executor that presents a PETSc SNESNGS object for use as a nonlinear preconditioner
 * on a single monolithic nonlinear system.  Attach it to a NewtonSNESExecutor via
 * nl_preconditioning; PETSc will propagate the outer SNES's DM to the SNESNGS SNES
 * and use its default secant NGS sweep as the preconditioner.
 *
 * This mirrors the -npc_snes_type ngs usage shown in PETSc's snes/tutorials/ex15.c.
 */
class NGSSNESExecutor : public SNESExecutor
{
public:
  static InputParameters validParams();
  NGSSNESExecutor(const InputParameters & params);

  virtual Result run() override;

protected:
  virtual void setupSNES() override;
};
