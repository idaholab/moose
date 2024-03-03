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
 * Pressure boundary condition using coupled variable to apply pressure in a given direction
 */
class CoupledPressureBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  CoupledPressureBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Will hold 0, 1, or 2 corresponding to x, y, or z.
  const unsigned int _component;
  /// The values of pressure to be imposed
  const VariableValue & _pressure;
};
