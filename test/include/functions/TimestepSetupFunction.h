//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

class TimestepSetupFunction : public Function
{
public:
  static InputParameters validParams();

  TimestepSetupFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const;

  virtual void timestepSetup();

private:
  int _local_timestep;
};
