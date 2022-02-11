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
 * Base class for computing the total energy for heat structures
 */
class HeatStructureEnergyBase : public ElementIntegralPostprocessor
{
public:
  HeatStructureEnergyBase(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Number of units that heat structure is multiplied by
  const Real _n_units;

  /// Reference temperature
  const Real & _T_ref;

  /// Density of the heat structure
  const MaterialProperty<Real> & _rho;

  /// Isobaric specific heat capacity
  const MaterialProperty<Real> & _cp;

  /// Temperature variable
  MooseVariable * _T_var;
  /// Temperature variable value
  const VariableValue & _T;

public:
  static InputParameters validParams();
};
