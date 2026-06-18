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
 * Provides Dirichlet boundary values for Kokkos linear finite volume advection-diffusion kernels.
 * The boundary value is supplied through a Kokkos-compatible functor. It is the Kokkos analog of
 * LinearFVAdvectionDiffusionFunctorDirichletBC.
 */
class KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
  : public Moose::Kokkos::LinearFVBoundaryCondition
{
public:
  static InputParameters validParams();

  KokkosLinearFVAdvectionDiffusionFunctorDirichletBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const FVDatum & datum) const;

private:
  KOKKOS_FUNCTION Real faceConductance(const FVDatum & datum) const;

  /// Diffusion coefficient (same functor as the paired diffusion kernel)
  const Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _diffusion_coeff;
  /// The functor providing the Dirichlet value on the boundary
  const Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _functor;
};

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::computeMatrixContribution(
    const FVDatum & datum) const
{
  return faceConductance(datum);
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::computeRightHandSideContribution(
    const FVDatum & datum) const
{
  const auto face_centroid = datum.faceCentroid();
  return faceConductance(datum) * _functor->value(_t, face_centroid);
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::faceConductance(const FVDatum & datum) const
{
  const auto face_centroid = datum.faceCentroid();
  return _diffusion_coeff->value(_t, face_centroid) * datum.faceArea() / datum.faceDCFMag();
}
