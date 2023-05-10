//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionSideIntegral.h"

/**
 * Integrates a heat flux function over a boundary.
 */
class HeatRateHeatFlux : public FunctionSideIntegral
{
public:
  static InputParameters validParams();

  HeatRateHeatFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Function by which to scale the heat flux
  const Function & _scale_fn;
};
