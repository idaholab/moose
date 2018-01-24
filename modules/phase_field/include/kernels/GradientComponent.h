//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GRADIENTCOMPONENT_H
#define GRADIENTCOMPONENT_H

#include "Kernel.h"

class GradientComponent;

template <>
InputParameters validParams<GradientComponent>();

class GradientComponent : public Kernel
{
public:
  GradientComponent(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Identity of the coupled variable
  const unsigned int _v_var;

  /// Gradient of the coupled variable
  const VariableGradient & _grad_v;

  /// Component of the gradient vector to match
  const unsigned int _component;
};

#endif // GRADIENTCOMPONENT_H
