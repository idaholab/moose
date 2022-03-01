//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

/**
 * Sets a constant value on a scalar variable
 */
class ConstantScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  ConstantScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const Real & _value;
};
