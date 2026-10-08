//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosRadiativeHeatFluxBCBase.h"

/**
 * Kokkos boundary condition for radiative heat exchange with a cylinder, where the boundary is
 * approximated as a cylinder as well
 */
class KokkosInfiniteCylinderRadiativeBC : public KokkosRadiativeHeatFluxBCBase
{
public:
  static InputParameters validParams();

  KokkosInfiniteCylinderRadiativeBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real coefficient(const unsigned int /* qp */, AssemblyDatum & /* datum */) const;

private:
  /// Effective emissivity of the radiative exchange between the two cylinders
  const Real _coefficient;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosInfiniteCylinderRadiativeBC::coefficient(const unsigned int /* qp */,
                                               AssemblyDatum & /* datum */) const
{
  return _coefficient;
}
