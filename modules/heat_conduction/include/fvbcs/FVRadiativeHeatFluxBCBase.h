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
class FVRadiativeHeatFluxBCBase : public FVFluxBC
{
public:
  static InputParameters validParams();

  FVRadiativeHeatFluxBCBase(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  /**
   * qdot = sigma * coeff * (T^4 - Tinf^4 )
   * sigma: _sigma_stefan_boltzmann
   * coeff: coefficient()
   * coefficientBody: cbody
   * Tinf: temperature of the body irhs
   */
  virtual Real coefficient() const = 0;

  /// temperature variable
  const ADVariableValue & _T;

  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;

  /// Function describing the temperature of the body completely surrounding the surface, e.g. the
  /// temperature of the body we are in radiative heat transfer with
  const Function & _tinf;

  /// Emissivity of the boundary
  const Real _eps_boundary;
};
