//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructure2DCouplerBCBase.h"

/**
 * Applies BC for HeatStructure2DRadiationCouplerRZ
 */
class HeatStructure2DRadiationCouplerRZBC : public HeatStructure2DCouplerBCBase
{
public:
  HeatStructure2DRadiationCouplerRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Emissivity of this boundary
  const Real & _emissivity;
  /// Emissivity of the coupled boundary
  const Real & _coupled_emissivity;
  /// View factor of this boundary
  const Real & _view_factor;
  /// Perimeter of this boundary
  const Real & _perimeter;
  /// Perimeter of the coupled boundary
  const Real & _coupled_perimeter;
  /// Stefan-Boltzmann constant
  const Real & _sigma;
  /// Radiation resistance
  const Real _radiation_resistance;

public:
  static InputParameters validParams();
};
