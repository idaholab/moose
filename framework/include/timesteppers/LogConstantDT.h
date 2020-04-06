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

/** Simple time-stepper which imposes a time step constant in the logarithmic
 * space */
class LogConstantDT : public TimeStepper
{
public:
  static InputParameters validParams();

  LogConstantDT(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

private:
  /// distance between two time steps in the logarithmic space
  const Real _log_dt;

  /// first time step (in absolute time)
  const Real _first_dt;

  const Real _dt_factor;

  const Real _growth_factor;
};
