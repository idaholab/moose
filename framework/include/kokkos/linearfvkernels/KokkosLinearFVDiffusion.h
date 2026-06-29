//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVKernel.h"
#include "KokkosParsedFunction.h"

/**
 * Kokkos linear finite volume flux kernel implementing the diffusion term. It is the Kokkos analog
 * of LinearFVDiffusion.
 */
class KokkosLinearFVDiffusion : public Moose::Kokkos::LinearFVFluxKernel
{
public:
  static InputParameters validParams();

  KokkosLinearFVDiffusion(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeNeighborMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const FVDatum & datum) const;

private:
  KOKKOS_FUNCTION Real faceConductance(const FVDatum & datum) const;

  /// Diffusion coefficient
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _diffusion_coeff;
};

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::computeMatrixContribution(const FVDatum & datum) const
{
  return datum.hasNeighbor() ? faceConductance(datum)
                             : _bc_data.matrix_coeff(datum.side(), datum.elemID());
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::computeNeighborMatrixContribution(const FVDatum & datum) const
{
  KOKKOS_ASSERT(datum.hasNeighbor());
  return -faceConductance(datum);
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::computeRightHandSideContribution(const FVDatum & datum) const
{
  return datum.hasNeighbor() ? 0 : _bc_data.rhs_coeff(datum.side(), datum.elemID());
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::faceConductance(const FVDatum & datum) const
{
  const auto face_centroid = datum.faceCentroid();
  const auto d_mag = datum.hasNeighbor() ? datum.faceDCNMag() : datum.faceDCFMag();
  return _diffusion_coeff->value(_t, face_centroid) * datum.faceArea() / d_mag;
}
