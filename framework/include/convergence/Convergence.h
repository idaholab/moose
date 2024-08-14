//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "SetupInterface.h"
#include "PerfGraphInterface.h"

/**
 * The Convergence class is designed to provide an interface for convergence related queries. It
 * defines basic routines for checking convergence, setting performance timers in the
 * computationally heavy sections of the code, and setting up the initial conditions for the
 * convergence check.
 */
class Convergence : public MooseObject, public SetupInterface, public PerfGraphInterface
{
public:
  static InputParameters validParams();

  enum class MooseConvergenceStatus
  {
    ITERATING = 0,
    CONVERGED = 2,
    DIVERGED = -2
  };

  Convergence(const InputParameters & parameters);

  virtual void initialSetup() override{};

  virtual MooseConvergenceStatus checkConvergence() = 0;

protected:
  PerfID _perf_check_convergence;
};
