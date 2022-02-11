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
 * Integrates a radiative heat flux over a boundary.
 */
class HeatRateRadiation : public SideIntegralPostprocessor
{
public:
  HeatRateRadiation(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Ambient temperature
  const Function & _T_ambient;
  /// Emissivity
  const Real & _emissivity;
  /// View factor function
  const Function & _view_factor_fn;
  /// Stefan-Boltzmann constant
  const Real & _sigma_stefan_boltzmann;
  /// Factor by which to scale integral, like when using a 2D domain
  const Real & _scale;

public:
  static InputParameters validParams();
};
