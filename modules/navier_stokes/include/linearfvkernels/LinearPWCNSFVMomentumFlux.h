//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearWCNSFVMomentumFlux.h"

class PorousRhieChowMassFlux;

/**
 * Momentum flux kernel for porous equations written in superficial velocity.
 *
 * The advective contribution is divided by the porosity on each side of a face. Internal baffle
 * faces advect the one-sided face states of the porous Rhie-Chow object and use a stress
 * discretization consistent with the velocity jump. Elsewhere, the stress contribution may use a
 * two-point transmissibility.
 */
class LinearPWCNSFVMomentumFlux : public LinearWCNSFVMomentumFlux
{
public:
  static InputParameters validParams();

  /**
   * Construct the porous momentum flux kernel.
   * @param params The input parameters for the kernel
   */
  LinearPWCNSFVMomentumFlux(const InputParameters & params);

  /// Assemble the internal-face matrix with side-specific porosity scaling of advection.
  void addMatrixContribution() override;

  /// Compute the element-row internal-face right-hand-side contribution.
  Real computeElemRightHandSideContribution() override;

  /// Compute the neighbor-row internal-face right-hand-side contribution.
  Real computeNeighborRightHandSideContribution() override;

protected:
  /// Compute the stress transmissibility, optionally using the two-point harmonic form.
  Real computeInternalStressTransmissibility() const override;

  /// Compute the explicit nonorthogonal and deviatoric corrections to the two-point stress.
  Real computeInternalStressExplicitCorrection() const override;

  /// Compute the boundary advection matrix contribution with local porosity scaling.
  Real computeAdvectionBoundaryMatrixContribution(const LinearFVAdvectionDiffusionBC * bc) override;

  /// Compute the boundary advection right-hand-side contribution with local porosity scaling.
  Real computeAdvectionBoundaryRHSContribution(const LinearFVAdvectionDiffusionBC * bc) override;

private:
  /// Porous Rhie-Chow object supplying porosity and pressure-baffle data.
  const PorousRhieChowMassFlux & _porous_mass_flux_provider;

  struct TwoPointStressData
  {
    Real transmissibility;
    Real elem_distance;
    Real neighbor_distance;
  };

  /// Whether to use a two-point harmonic transmissibility for the stress term.
  const bool _use_two_point_stress_transmissibility;

  /// Compute the geometry and material data used by the harmonic stress discretization.
  TwoPointStressData twoPointStressData() const;

  /// Whether the current face is an internal face represented by a pressure-jump model.
  bool isInternalBaffleFace() const;

  /**
   * Compute the correction that replaces the shared advected state with the baffle face state:
   * the upstream interstitial tangential velocity and this side's extrapolated normal velocity.
   * @param elem_side Whether to compute the correction for the element side
   */
  Real computeBaffleAdvectionExplicitCorrection(bool elem_side) const;

  /// Implicit coefficients and deferred right-hand sides of the stress on a baffle face.
  struct BaffleStressData
  {
    /// Coefficients of the element row on the element and neighbor values.
    Real elem_elem;
    Real elem_neighbor;
    /// Coefficients of the neighbor row on the element and neighbor values.
    Real neighbor_elem;
    Real neighbor_neighbor;
    /// Explicit parts of the element and neighbor rows.
    Real elem_rhs;
    Real neighbor_rhs;
  };

  /**
   * Compute the stress on the current baffle face, per unit face area. The interstitial
   * tangential velocity is continuous and carries one shear traction, while each side evaluates
   * its normal stress against its own face normal velocity.
   */
  BaffleStressData baffleStressData() const;

  /**
   * Return the inverse porosity on one side of the current face.
   * @param elem_side Whether to evaluate porosity on the element side
   */
  Real inversePorosity(bool elem_side) const;
};
