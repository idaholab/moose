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
 * Executor implementing nonlinear Gauss-Seidel (SNESNGS) over a set of libMesh nonlinear
 * systems.  Each application of the NGS preconditioner sweeps through the listed inner
 * systems in order, calling _fe_problem.solve(sys_num) for each.
 *
 * NGSSNESExecutor can be used as a standalone solver or as the nl_preconditioning
 * for a NewtonSNESExecutor.  It requires a multi-system problem configuration.
 */
class NGSSNESExecutor : public SNESExecutor
{
public:
  static InputParameters validParams();
  NGSSNESExecutor(const InputParameters & params);

  virtual Result run() override;

protected:
  virtual void setupSNES() override;

private:
  std::vector<unsigned int> _inner_sys_nums;

  static PetscErrorCode ngsCallback(SNES snes, Vec x, Vec b, void * ctx);
};
