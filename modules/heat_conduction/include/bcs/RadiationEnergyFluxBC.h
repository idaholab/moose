//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class RadiationEnergyFluxBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  RadiationEnergyFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The Stefan-Boltzmann constant
  const MaterialProperty<Real> & _sb_constant;
  /// The absorptivity
  const MaterialProperty<Real> & _absorptivity;
  /// The far-field temperature
  const Real _ff_temp;
};
