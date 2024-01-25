

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDirichletBCBase.h"
#include "FVFluxBC.h"

/**
 * Applies a wall function to the turbulent pressure distribution function
 */
class INSFVTFWallFunctionBC : public FVDirichletBCBase
{
public:
  INSFVTFWallFunctionBC(const InputParameters & parameters);

  static InputParameters validParams();

  ADReal boundaryValue(const FaceInfo & fi) const override;

private:
  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;

  /// Turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _epsilon;

  /// Turbulent normal wall fluctuations
  const Moose::Functor<ADReal> & _v2;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// C_mu turbulent coefficient
  const Moose::Functor<ADReal> & _C_mu;

  /// Model constants
  const Real _n;
};
