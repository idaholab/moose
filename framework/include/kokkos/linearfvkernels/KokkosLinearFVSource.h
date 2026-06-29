//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVElementalKernel.h"
#include "KokkosParsedFunction.h"

/**
 * Kokkos linear finite volume elemental kernel that adds a scaled volumetric source to the
 * right-hand side. It is the Kokkos analog of LinearFVSource.
 */
class KokkosLinearFVSource : public Moose::Kokkos::LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  KokkosLinearFVSource(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const FVDatum & datum) const;

private:
  /// The source density
  const Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _source_density;
  /// Coefficient for multiplying the surface density
  const Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _scale;
};

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosLinearFVSource::computeRightHandSideContribution(const FVDatum & datum) const
{
  const auto centroid = datum.elementCentroid();
  return _scale->value(_t, centroid) * _source_density->value(_t, centroid) * datum.elementVolume();
}
