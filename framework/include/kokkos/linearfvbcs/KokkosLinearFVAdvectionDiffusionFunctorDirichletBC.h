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
#include "KokkosFunction.h"

class KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
  : public Moose::Kokkos::LinearFVBoundaryCondition
{
public:
  static InputParameters validParams();

  KokkosLinearFVAdvectionDiffusionFunctorDirichletBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeMatrixContribution(const AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const AssemblyDatum & datum) const;

private:
  KOKKOS_FUNCTION Real faceConductance(const AssemblyDatum & datum) const;

  /// Diffusion coefficient (same functor as the paired diffusion kernel)
  const Moose::Kokkos::Function _diffusion_coeff;
  /// The functor providing the Dirichlet value on the boundary
  const Moose::Kokkos::Function _functor;
};

KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::computeMatrixContribution(
    const AssemblyDatum & datum) const
{
  return faceConductance(datum);
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::computeRightHandSideContribution(
    const AssemblyDatum & datum) const
{
  const auto face_centroid = datum.mesh().getFaceCentroid(datum.elemID(), datum.side());
  return faceConductance(datum) * _functor.value(_t, face_centroid);
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::faceConductance(
    const AssemblyDatum & datum) const
{
  const auto face_centroid = datum.mesh().getFaceCentroid(datum.elemID(), datum.side());
  return _diffusion_coeff.value(_t, face_centroid) *
         datum.mesh().getFaceArea(datum.elemID(), datum.side()) /
         datum.mesh().getFaceDCFMag(datum.elemID(), datum.side());
}
