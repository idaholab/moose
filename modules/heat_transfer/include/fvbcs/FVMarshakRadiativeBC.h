//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Boundary condition for radiative heat flux where temperature and the
 * temperature of a body in radiative heat transfer are specified.
 */
class FVMarshakRadiativeBC : public FVFluxBC
{
public:
  static InputParameters validParams();

  FVMarshakRadiativeBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  /// The coupled functor applying the radiation temperature
  const Moose::Functor<ADReal> & _temperature_radiation;

  /// Diffusion coefficient
  const Moose::Functor<ADReal> & _coeff_diffusion;

  /// Emissivity of the boundary
  const Real _eps_boundary;
};
