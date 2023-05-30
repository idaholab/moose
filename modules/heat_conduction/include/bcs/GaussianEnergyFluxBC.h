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
 * Describes an incoming heat flux beam with a Gaussian profile. Form is taken from:
 *
 * @techreport{noble2007use,
 *   title={Use of Aria to simulate laser weld pool dynamics for neutron generator production.},
 *   author={Noble, David R and Notz, Patrick K and Martinez, Mario J and Kraynik, Andrew Michael},
 *   year={2007},
 *   institution={Sandia National Laboratories (SNL), Albuquerque, NM, and Livermore, CA}}
 */
class GaussianEnergyFluxBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  GaussianEnergyFluxBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// an effective radius used to specify the radial distribution of beam energy
  const Real _reff;
  /// the average heat flux of the beam
  const Real _F0;
  /// beam radius
  const Real _R;
  /// the x-coordinate of the beam center
  const Function & _x_beam_coord;
  /// the y-coordinate of the beam center
  const Function & _y_beam_coord;
  /// the z-coordiinate of the beam center
  const Function & _z_beam_coord;
};
