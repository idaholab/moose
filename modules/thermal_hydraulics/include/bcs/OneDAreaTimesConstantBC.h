//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OneDNodalBC.h"

/**
 *
 */
class OneDAreaTimesConstantBC : public OneDNodalBC
{
public:
  OneDAreaTimesConstantBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const Real & _value;
  const VariableValue & _area;

public:
  static InputParameters validParams();
};
