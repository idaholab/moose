//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"
#include "KokkosGrayLambertSurfaceRadiationBase.h"

/**
 * Kokkos boundary condition for radiative heat that is computed by the
 * KokkosGrayLambertSurfaceRadiationBase user object
 */
class KokkosGrayLambertNeumannBC : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosGrayLambertNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

private:
  /// Boundary ID of the current side that participates in the radiative exchange
  KOKKOS_FUNCTION BoundaryID boundaryID(const AssemblyDatum & datum) const;

  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;
  /// Surface radiation user object
  const Moose::Kokkos::VirtualUserObject<KokkosGrayLambertSurfaceRadiationBase> _glsr_uo;
  /// Whether to reconstruct the emission by the T^4 law instead of applying a constant flux
  const bool _reconstruct_emission;
  /// Boundaries of this boundary condition
  Moose::Kokkos::Array<BoundaryID> _boundary_ids;
};

KOKKOS_FUNCTION inline BoundaryID
KokkosGrayLambertNeumannBC::boundaryID(const AssemblyDatum & datum) const
{
  const auto & mesh = datum.mesh();

  for (unsigned int i = 0; i < _boundary_ids.size(); ++i)
    if (mesh.isSideOnBoundary(datum.elemID(), datum.side(), _boundary_ids[i]))
      return _boundary_ids[i];

  KOKKOS_ASSERT(false);
  return _boundary_ids[0];
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosGrayLambertNeumannBC::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  const auto id = boundaryID(datum);

  if (!_reconstruct_emission)
    return _glsr_uo.getSurfaceHeatFluxDensity(id);

  const Real u = _u(datum, qp);
  const Real eps = _glsr_uo.getSurfaceEmissivity(id);
  const Real emission = _sigma_stefan_boltzmann * u * u * u * u;

  return eps * (emission - _glsr_uo.getSurfaceIrradiation(id));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosGrayLambertNeumannBC::precomputeQpJacobian(const unsigned int j,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  // this is not the exact Jacobian but it ensures correct scaling
  const Real u = _u(datum, qp);

  return _sigma_stefan_boltzmann * _glsr_uo.getSurfaceEmissivity(boundaryID(datum)) * 4 * u * u *
         u * _phi(datum, j, qp);
}
