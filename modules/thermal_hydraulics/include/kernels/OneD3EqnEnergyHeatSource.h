//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class Function;

/**
 * Volumetric heat source for 1-phase flow channel
 */
class OneD3EqnEnergyHeatSource : public Kernel
{
public:
  OneD3EqnEnergyHeatSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Heat source function
  const Function & _q;
  /// Cross sectional area
  const VariableValue & _A;

public:
  static InputParameters validParams();
};
