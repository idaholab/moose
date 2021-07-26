//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadiativeHeatFluxBCBase.h"

/**
 * Boundary condition for radiative heat exchange with a cylinder, the outer
 * surface of the domain is assumed to be cylindrical as well
 */
template <bool is_ad>
class InfiniteCylinderRadiativeBCTempl : public RadiativeHeatFluxBCBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  InfiniteCylinderRadiativeBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> coefficient() const override;

  /// emissivity of the cylinder in radiative heat transfer with the boundary
  const Real _eps_cylinder;

  /// radius of the boundary
  const Real _boundary_radius;

  /// radius of the cylinder around the boundary
  const Real _cylinder_radius;

  /// coefficients are constant and pre-computed
  Real _coefficient;

  usingGenericIntegratedBCMembers;
};

typedef InfiniteCylinderRadiativeBCTempl<false> InfiniteCylinderRadiativeBC;
typedef InfiniteCylinderRadiativeBCTempl<true> ADInfiniteCylinderRadiativeBC;
