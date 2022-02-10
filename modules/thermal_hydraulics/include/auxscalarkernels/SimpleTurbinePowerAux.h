//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConstantScalarAux.h"

class SinglePhaseFluidProperties;

/**
 * Computes turbine power for 1-phase flow
 */
class SimpleTurbinePowerAux : public ConstantScalarAux
{
public:
  SimpleTurbinePowerAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Flag indicating if turbine is operating or not
  const bool & _on;

public:
  static InputParameters validParams();
};
