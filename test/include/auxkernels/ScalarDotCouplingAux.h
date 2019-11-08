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
 * Couples in the time derivatives of a scalar variable
 */
class ScalarDotCouplingAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ScalarDotCouplingAux(const InputParameters & parameters);
  virtual ~ScalarDotCouplingAux();

protected:
  virtual Real computeValue();

  const VariableValue & _v_dot;
};
