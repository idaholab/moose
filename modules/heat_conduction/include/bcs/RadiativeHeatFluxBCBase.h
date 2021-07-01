//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericIntegratedBC.h"

/**
 * Boundary condition for radiative heat flux where temperature and the
 * temperature of a body in radiative heat transfer are specified.
 */
template <bool is_ad>
class RadiativeHeatFluxBCBaseTempl : public GenericIntegratedBC<is_ad>
{
public:
  static InputParameters validParams();

  RadiativeHeatFluxBCBaseTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual();
  virtual Real computeQpJacobian();

  /**
   * qdot = sigma * coeff * (T^4 - Tinf^4 )
   * sigma: _sigma_stefan_boltzmann
   * coeff: coefficient()
   * coefficientBody: cbody
   * Tinf: temperature of the body irhs
   */
  virtual GenericReal<is_ad> coefficient() const = 0;

  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;

  /// Function describing the temperature of the body irhs
  const Function & _tinf;

  /// Emissivity of the boundary
  const Real _eps_boundary;

  usingGenericIntegratedBCMembers;
};

typedef RadiativeHeatFluxBCBaseTempl<false> RadiativeHeatFluxBCBase;
typedef RadiativeHeatFluxBCBaseTempl<true> ADRadiativeHeatFluxBCBase;
