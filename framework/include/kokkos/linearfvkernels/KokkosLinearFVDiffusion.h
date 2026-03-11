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

class KokkosLinearFVAdvectionDiffusionFunctorDirichletBC;

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
  KOKKOS_FUNCTION Real faceConductance(const AssemblyDatum & datum) const
  {
    const auto face_centroid = datum.mesh().getFaceCentroid(datum.elemID(), datum.side());
    return _diffusion_coeff.value(_t, face_centroid) *
           datum.mesh().getFaceArea(datum.elemID(), datum.side()) /
           datum.mesh().getFaceDCNMag(datum.elemID(), datum.side());
  }

  KOKKOS_FUNCTION int dirichletIndex(const BoundaryID boundary_id) const
  {
    for (std::size_t i = 0; i < _dirichlet_boundary_ids.size(); ++i)
      if (_dirichlet_boundary_ids[i] == boundary_id)
        return static_cast<int>(i);

    return -1;
  }

  const Moose::Kokkos::Function _diffusion_coeff;
  const bool _use_nonorthogonal_correction;
  Moose::Kokkos::Array<BoundaryID> _dirichlet_boundary_ids;
  Moose::Kokkos::Array<Moose::Kokkos::Function> _dirichlet_values;
};

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::computeMatrixContribution(const AssemblyDatum & datum) const
{
  if (datum.hasNeighbor())
    return faceConductance(datum);

  const auto boundary_id = datum.mesh().getFaceBoundaryID(datum.elemID(), datum.side());
  return dirichletIndex(boundary_id) >= 0 ? faceConductance(datum) : 0;
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
  const auto index = dirichletIndex(boundary_id);
  if (index < 0)
    return 0;

  return faceConductance(datum) *
         _dirichlet_values[index].value(_t,
                                        datum.mesh().getFaceCentroid(datum.elemID(), datum.side()));
}
