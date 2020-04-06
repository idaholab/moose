//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 *
 */
class MomentumFreeBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  MomentumFreeBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  unsigned int _component;

  const VariableValue & _pressure;
  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;
};
