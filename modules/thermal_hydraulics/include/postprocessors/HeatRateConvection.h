//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * Integrates a convective heat flux over a boundary.
 */
class HeatRateConvection : public SideIntegralPostprocessor
{
public:
  HeatRateConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Ambient temperature function
  const Function & _T_ambient_fn;
  /// Ambient heat transfer coefficient function
  const Function & _htc_ambient_fn;
  /// Function by which to scale the heat flux
  const Function & _scale_fn;

public:
  static InputParameters validParams();
};
