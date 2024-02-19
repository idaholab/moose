//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Couple an array variable, and return a component of its gradient of time derivative
 */
class CoupledArrayGradDotAux : public ArrayAuxKernel
{
public:
  static InputParameters validParams();

  CoupledArrayGradDotAux(const InputParameters & parameters);

protected:
  RealEigenVector computeValue() override;

  const ArrayVariableValue & _v;

  const ArrayVariableGradient & _v_grad_dot;

  const unsigned int _grad_component;
};
