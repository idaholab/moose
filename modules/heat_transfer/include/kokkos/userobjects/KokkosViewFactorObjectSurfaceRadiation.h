//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosGrayLambertSurfaceRadiationBase.h"

/**
 * Kokkos user object computing radiative heat transfer between side sets, where the view factors
 * are computed by a ViewFactor object
 */
class KokkosViewFactorObjectSurfaceRadiation : public KokkosGrayLambertSurfaceRadiationBase
{
public:
  static InputParameters validParams();

  KokkosViewFactorObjectSurfaceRadiation(const InputParameters & parameters);

  ///@{ device interface of this user object
  KOKKOS_FUNCTION Real getSurfaceIrradiation(BoundaryID id) const { return surfaceIrradiation(id); }
  KOKKOS_FUNCTION Real getSurfaceHeatFluxDensity(BoundaryID id) const;
  KOKKOS_FUNCTION Real getSurfaceTemperature(BoundaryID id) const { return surfaceTemperature(id); }
  KOKKOS_FUNCTION Real getSurfaceRadiosity(BoundaryID id) const { return surfaceRadiosity(id); }
  KOKKOS_FUNCTION Real getSurfaceEmissivity(BoundaryID id) const { return surfaceEmissivity(id); }
  ///@}

protected:
  virtual std::vector<std::vector<Real>> setViewFactors() override;
};

KOKKOS_FUNCTION inline Real
KokkosViewFactorObjectSurfaceRadiation::getSurfaceHeatFluxDensity(BoundaryID id) const
{
  return surfaceHeatFluxDensity(id);
}
