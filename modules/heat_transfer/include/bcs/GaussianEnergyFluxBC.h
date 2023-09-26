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

/**
 * Describes an incoming heat flux beam with a Gaussian profile. Form is taken from
 * https://en.wikipedia.org/wiki/Gaussian_beam
 */
class GaussianEnergyFluxBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  GaussianEnergyFluxBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// the total power of the beam
  const Real _P0;
  /// beam radius, specifically the radius at which the beam intensity falls to $1/e^2$ of its axial
  /// value
  const Real _R;
  /// the x-coordinate of the beam center
  const Function & _x_beam_coord;
  /// the y-coordinate of the beam center
  const Function & _y_beam_coord;
  /// the z-coordiinate of the beam center
  const Function & _z_beam_coord;
};
