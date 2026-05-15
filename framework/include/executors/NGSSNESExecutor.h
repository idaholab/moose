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
 * Executor that presents a PETSc SNESNGS object. PETSc will apply a default secant NGS sweep
 */
class NGSSNESExecutor : public SNESExecutor
{
public:
  static InputParameters validParams();
  NGSSNESExecutor(const InputParameters & params);

  virtual Result run() override;

protected:
  virtual void setupSNES() override;

  /// The nonlinear system this SNES will be applied to
  const unsigned int _nl_sys_num;
};
