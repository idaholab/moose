//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PRESSUREGRADIENT_H
#define PRESSUREGRADIENT_H

#include "Kernel.h"

class PressureGradient;

template <>
InputParameters validParams<PressureGradient>();

/**
 *
 */
class PressureGradient : public Kernel
{
public:
  PressureGradient(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  unsigned int _component;
  const VariableValue & _pressure;
};

#endif /* PRESSUREGRADIENT_H */
