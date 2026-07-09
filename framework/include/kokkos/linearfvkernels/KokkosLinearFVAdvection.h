//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVFluxKernel.h"

/**
 * Kokkos linear finite volume flux kernel implementing first-order upwind advection with a
 * constant velocity. It is the Kokkos analog of LinearFVAdvection for the upwind interpolation
 * case.
 */
class KokkosLinearFVAdvection : public Moose::Kokkos::LinearFVFluxKernel
{
public:
  static InputParameters validParams();

  KokkosLinearFVAdvection(const InputParameters & parameters);

  virtual bool needsBoundaryValueData() const override { return true; }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeNeighborMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const FVDatum & datum) const;

private:
  /// Normal advective face flux
  KOKKOS_FUNCTION Real normalFaceFlux(const FVDatum & datum) const;

  /// Constant advecting velocity vector
  const Moose::Kokkos::Real3 _velocity;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVAdvection::computeMatrixContribution(const FVDatum & datum) const
{
  const auto face_flux = normalFaceFlux(datum);

  if (hasFaceNeighbor(datum))
    return face_flux > 0 ? face_flux * datum.faceArea() : 0;

  return face_flux * datum.faceArea() * boundaryValueRelation(datum).coefficient;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVAdvection::computeNeighborMatrixContribution(const FVDatum & datum) const
{
  KOKKOS_ASSERT(hasFaceNeighbor(datum));

  const auto face_flux = normalFaceFlux(datum);
  return face_flux < 0 ? face_flux * datum.faceArea() : 0;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVAdvection::computeRightHandSideContribution(const FVDatum & datum) const
{
  if (hasFaceNeighbor(datum))
    return 0;

  return -normalFaceFlux(datum) * datum.faceArea() * boundaryValueRelation(datum).source;
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvection::normalFaceFlux(const FVDatum & datum) const
{
  return _velocity * datum.faceNormal();
}
