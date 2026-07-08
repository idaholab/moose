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
  /**
   * Method for evaluating the face diffusion coefficient times the face area divided by the length
   * between the cell center and face center. The name "conductance" is used here for this finite
   * volume face coefficient because heat conduction problems provide a thermal conductivity as the
   * diffusion coefficient, which has conductance units after multiplying by area over length.
   * Similar "diffusive conductance" terminology is also used in mass transport contexts; see
   * https://www.goldsim.com/Courses/ContaminantTransport/Unit8/Lesson3/
   * @param datum Finite volume datum used to query the face area and cell-center-to-face-center
   * distance
   * @param face_centroid The xyz coordinates of the face centroid. This is parameter instead of
   * being queried from the \p datum in order to save a global memory lookup in the RHS computation
   * call-chain
   */
  KOKKOS_FUNCTION Real faceConductance(const FVDatum & datum,
                                       const Moose::Kokkos::Real3 face_centroid) const;

  /// Diffusion coefficient (same functor as the paired diffusion kernel)
  const Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _diffusion_coeff;
  /// The functor providing the Dirichlet value on the boundary
  const Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _functor;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::computeMatrixContribution(
    const FVDatum & datum) const
{
  return faceConductance(datum, datum.faceCentroid());
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::computeRightHandSideContribution(
    const FVDatum & datum) const
{
  const auto face_centroid = datum.faceCentroid();
  return faceConductance(datum, face_centroid) * _functor->value(_t, face_centroid);
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::faceConductance(
    const FVDatum & datum, const Moose::Kokkos::Real3 face_centroid) const
{
  return _diffusion_coeff->value(_t, face_centroid) * datum.faceArea() / datum.faceDCFMag();
}
