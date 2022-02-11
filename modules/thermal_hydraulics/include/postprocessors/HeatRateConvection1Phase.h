//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

/**
 * Computes convective heat rate into a 1-phase flow channel
 */
class HeatRateConvection1Phase : public ElementIntegralPostprocessor
{
public:
  HeatRateConvection1Phase(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// Wall temperature
  const MaterialProperty<Real> & _T_wall;
  /// Fluid temperature
  const MaterialProperty<Real> & _T;
  /// Convective heat transfer coefficient, W/m^2-K
  const MaterialProperty<Real> & _Hw;
  /// Heat flux perimeter
  const VariableValue & _P_hf;

public:
  static InputParameters validParams();
};
