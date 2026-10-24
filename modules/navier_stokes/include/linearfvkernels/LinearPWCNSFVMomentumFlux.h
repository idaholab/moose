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
 * faces advect the upstream interstitial tangential velocity with each side's normal velocity and
 * transmit only a shear traction that depends on which sides are clear fluid.
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
  /// Compute the boundary advection matrix contribution with local porosity scaling.
  Real computeAdvectionBoundaryMatrixContribution(const LinearFVAdvectionDiffusionBC * bc) override;

  /// Compute the boundary advection right-hand-side contribution with local porosity scaling.
  Real computeAdvectionBoundaryRHSContribution(const LinearFVAdvectionDiffusionBC * bc) override;

private:
  /// Porous Rhie-Chow object supplying porosity and pressure-baffle data.
  const PorousRhieChowMassFlux & _porous_mass_flux_provider;

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
   * Compute the stress on the current baffle face, per unit face area. Only a shear traction is
   * transmitted: between two clear sides with the two-point transmissibility, from a
   * clear side against the superficial cell velocity of a porous side, and none between two
   * porous sides.
   */
  BaffleStressData baffleStressData() const;

  /**
   * Whether one side of the current face is clear fluid, i.e. has unit porosity.
   * @param elem_side Whether to check the element side
   */
  bool isClearFluidSide(bool elem_side) const;

  /**
   * Return the inverse porosity on one side of the current face.
   * @param elem_side Whether to evaluate porosity on the element side
   */
  Real inversePorosity(bool elem_side) const;
};
