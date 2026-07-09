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

  virtual bool needsBoundaryNormalGradientData() const override { return true; }
  virtual bool hasInternalRightHandSideContribution() const override { return false; }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeInternalMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeInternalNeighborMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeBoundaryMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeBoundaryRightHandSideContribution(const FVDatum & datum) const;

private:
  /**
   * Method for evaluating the face diffusion coefficient times the face area divided by the
   * distance between cell centers. The name "conductance" is used here for this finite volume face
   * coefficient because heat conduction problems provide a thermal conductivity as the diffusion
   * coefficient, which has conductance units after multiplying by area over length. Similar
   * "diffusive conductance" terminology is also used in mass transport contexts; see
   * https://www.goldsim.com/Courses/ContaminantTransport/Unit8/Lesson3/
   */
  KOKKOS_FUNCTION Real faceConductance(const FVDatum & datum) const;

  /// Method for evaluating the face diffusion coefficient
  KOKKOS_FUNCTION Real faceDiffusionCoefficient(const FVDatum & datum) const;

  /// Diffusion coefficient
  Moose::Kokkos::ReferenceWrapper<const KokkosParsedFunction> _diffusion_coeff;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVDiffusion::computeInternalMatrixContribution(const FVDatum & datum) const
{
  return faceConductance(datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVDiffusion::computeInternalNeighborMatrixContribution(const FVDatum & datum) const
{
  return -faceConductance(datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVDiffusion::computeBoundaryMatrixContribution(const FVDatum & datum) const
{
  return -faceDiffusionCoefficient(datum) * datum.faceArea() *
         boundaryNormalGradientRelation(datum).coefficient;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosLinearFVDiffusion::computeBoundaryRightHandSideContribution(const FVDatum & datum) const
{
  return faceDiffusionCoefficient(datum) * datum.faceArea() *
         boundaryNormalGradientRelation(datum).source;
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::faceConductance(const FVDatum & datum) const
{
  return faceDiffusionCoefficient(datum) * datum.faceArea() / datum.faceDCNMag();
}

KOKKOS_FUNCTION inline Real
KokkosLinearFVDiffusion::faceDiffusionCoefficient(const FVDatum & datum) const
{
  return _diffusion_coeff->value(_t, datum.faceCentroid());
}
