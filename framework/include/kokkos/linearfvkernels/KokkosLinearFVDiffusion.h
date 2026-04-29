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
#include "KokkosFunction.h"

class KokkosLinearFVDiffusion : public Moose::Kokkos::LinearFVFluxKernel
{
public:
  static InputParameters validParams();

  KokkosLinearFVDiffusion(const InputParameters & parameters);

  virtual void initialSetup() override;

  KOKKOS_FUNCTION Real computeMatrixContribution(const AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeNeighborMatrixContribution(const AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeRightHandSideContribution(const AssemblyDatum & datum) const;

private:
  KOKKOS_FUNCTION Real faceConductance(const AssemblyDatum & datum) const;

  /// Diffusion coefficient
  const Moose::Kokkos::Function _diffusion_coeff;
};

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::computeMatrixContribution(const AssemblyDatum & datum) const
{
  if (datum.hasNeighbor())
    return faceConductance(datum);

  const auto boundary_id = datum.mesh().getFaceBoundaryID(datum.elemID(), datum.side());
  return _kokkos_var.hasBoundaryCondition(boundary_id) ? faceConductance(datum) : 0;
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::computeNeighborMatrixContribution(const AssemblyDatum & datum) const
{
  return datum.hasNeighbor() ? -faceConductance(datum) : 0;
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::computeRightHandSideContribution(const AssemblyDatum & datum) const
{
  if (datum.hasNeighbor())
    return 0;

  const auto boundary_id = datum.mesh().getFaceBoundaryID(datum.elemID(), datum.side());
  const auto index = _kokkos_var.boundaryConditionIndex(boundary_id);
  if (index < 0)
    return 0;

  return faceConductance(datum) *
         _kokkos_var.boundaryConditionValue(index).value(
             _t, datum.mesh().getFaceCentroid(datum.elemID(), datum.side()));
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::faceConductance(const AssemblyDatum & datum) const
{
  const auto face_centroid = datum.mesh().getFaceCentroid(datum.elemID(), datum.side());
  const auto d_mag = datum.hasNeighbor()
                         ? datum.mesh().getFaceDCNMag(datum.elemID(), datum.side())
                         : datum.mesh().getFaceDCFMag(datum.elemID(), datum.side());
  return _diffusion_coeff.value(_t, face_centroid) *
         datum.mesh().getFaceArea(datum.elemID(), datum.side()) / d_mag;
}
