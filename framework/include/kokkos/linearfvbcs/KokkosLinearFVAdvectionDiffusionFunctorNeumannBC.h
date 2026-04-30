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

class KokkosLinearFVAdvectionDiffusionFunctorNeumannBC
  : public Moose::Kokkos::LinearFVBoundaryCondition
{
public:
  static InputParameters validParams();

  KokkosLinearFVAdvectionDiffusionFunctorNeumannBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeMatrixContribution(const AssemblyDatum &) const { return 0; }
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const AssemblyDatum & datum) const;

private:
  /// The functor providing the outward normal flux on the boundary (positive = outflow)
  const Moose::Kokkos::Function _flux_functor;
};

KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorNeumannBC::computeRightHandSideContribution(
    const AssemblyDatum & datum) const
{
  const auto face_centroid = datum.mesh().getFaceCentroid(datum.elemID(), datum.side());
  return _flux_functor.value(_t, face_centroid) *
         datum.mesh().getFaceArea(datum.elemID(), datum.side());
}
