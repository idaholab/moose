//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Describes an incoming heat flux beam with a Gaussian profile. Form is taken from
 * https://en.wikipedia.org/wiki/Gaussian_beam
 */
class FVGaussianEnergyFluxBC : public FVFluxBC
{
public:
  static InputParameters validParams();

  FVGaussianEnergyFluxBC(const InputParameters & params);

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

  friend class GaussianEnergyFluxBC;
};
