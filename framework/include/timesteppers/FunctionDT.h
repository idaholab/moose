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

  const std::vector<Real> & _time_t;
  const std::vector<Real> & _time_dt;

  /// true, if we are using `_function`, false if we are using _time_ipol
  bool _use_function;
  /// The time-dependent function specifying the time step size (turn this into a reference then
  /// `time_t` and `time_dt` is removed)
  const Function * _function;

  /// Piecewise linear definition of time stepping
  std::unique_ptr<LinearInterpolation> _time_ipol;

  Real _growth_factor;
  Real _min_dt;

  /// Whether or not to interpolate DT between times
  bool _interpolate;

  std::vector<Real> _time_knots;
};
