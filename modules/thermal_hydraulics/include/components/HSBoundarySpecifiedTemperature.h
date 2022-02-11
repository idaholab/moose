//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HSBoundary.h"

/**
 * Boundary condition to set a specified value of temperature in a heat structure
 */
class HSBoundarySpecifiedTemperature : public HSBoundary
{
public:
  HSBoundarySpecifiedTemperature(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// The function prescribing the temperature at the boundary
  const FunctionName & _T_func;

public:
  static InputParameters validParams();
};
