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

class TestSourceStepper : public TimeStepper
{
public:
  static InputParameters validParams();

  TestSourceStepper(const InputParameters & parameters);

  /**
   * This gets called when time step is rejected for all input time steppers
   */
  virtual void rejectStep() override;

protected:
  virtual Real computeDT() override { return _dt; };
  virtual Real computeInitialDT() override { return _dt; };

  Real _dt;
};
