//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"
#include "FunctionInterface.h"
#include "LinearInterpolation.h"

class Function;

class FunctionDT : public TimeStepper, public FunctionInterface
{
public:
  static InputParameters validParams();

  FunctionDT(const InputParameters & parameters);

  virtual void init() override;

  virtual void postStep() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  void removeOldKnots();

  /// The time-dependent function specifying the time step size
  const Function & _function;

  Real _growth_factor;
  Real _min_dt;

  std::vector<Real> _time_knots;
};
