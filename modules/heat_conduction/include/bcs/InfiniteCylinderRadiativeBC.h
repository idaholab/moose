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

class InfiniteCylinderRadiativeBC;

template <>
InputParameters validParams<InfiniteCylinderRadiativeBC>();

/**
 * Boundary condition for radiative heat exchange with a cylinder, the outer
 * surface of the domain is assumed to be cylindrical as well
 */
class InfiniteCylinderRadiativeBC : public RadiativeHeatFluxBCBase
{
public:
  InfiniteCylinderRadiativeBC(const InputParameters & parameters);

protected:
  virtual Real coefficient() const override;

  /// emissivity of the cylinder irht with the boundary
  const Real _eps_cylinder;

  /// radius of the boundary
  const Real _boundary_radius;

  /// radius of the cylinder around the boundary
  const Real _cylinder_radius;

  /// coefficients are constant and pre-computed
  Real _coefficient;
};

