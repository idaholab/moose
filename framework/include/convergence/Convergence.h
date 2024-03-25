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

class Convergence : public MooseObject, public SetupInterface, public PerfGraphInterface
{
public:
  static InputParameters validParams();

  enum class MooseAlgebraicConvergence
  {
    ITERATING = 0,
    CONVERGED = 2,
    DIVERGED = -2
  };

  Convergence(const InputParameters & parameters);

  static InputParameters commonParams();

  virtual void initialSetup(){};

  virtual void timestepSetup(){};

  virtual MooseAlgebraicConvergence
  checkAlgebraicConvergence(int it, Real xnorm, Real snorm, Real fnorm) = 0;

protected:
  PerfID _perf_nonlinear;
  /**
   * Performs setup necessary for each call to checkAlgebraicConvergence
   */
  virtual void nonlinearConvergenceSetup(){};
};
