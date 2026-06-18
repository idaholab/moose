//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVBoundaryCondition.h"
#include "KokkosParsedFunction.h"

/**
 * Applies a prescribed outward normal flux on a boundary for Kokkos linear finite volume
 * advection-diffusion kernels. It is the Kokkos analog of
 * LinearFVAdvectionDiffusionFunctorNeumannBC.
 */
class KokkosLinearFVAdvectionDiffusionFunctorNeumannBC
  : public Moose::Kokkos::LinearFVBoundaryCondition
{
public:
  static InputParameters validParams();

  KokkosLinearFVAdvectionDiffusionFunctorNeumannBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContribution(const FVDatum &) const
  {
    return 0;
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const FVDatum & datum) const;

private:
  /// The functor providing the outward normal flux on the boundary (positive = outflow)
  const Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _flux_functor;
};

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorNeumannBC::computeRightHandSideContribution(
    const FVDatum & datum) const
{
  const auto face_centroid = datum.faceCentroid();
  return _flux_functor->value(_t, face_centroid) * datum.faceArea();
}
