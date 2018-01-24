//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TIMESTEPSETUPFUNCTION_H
#define TIMESTEPSETUPFUNCTION_H

#include "Function.h"

class TimestepSetupFunction;

template <>
InputParameters validParams<TimestepSetupFunction>();

class TimestepSetupFunction : public Function
{
public:
  TimestepSetupFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);

  virtual void timestepSetup();

private:
  unsigned int & _local_timestep;
};

#endif // TIMESTEPSETUPFUNCTION_H
