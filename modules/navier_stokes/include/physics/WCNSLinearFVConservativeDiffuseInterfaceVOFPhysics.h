//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSLinearFVScalarTransportPhysics.h"

/**
 * Creates the volume-fraction transport of a two-phase diffuse-interface (volume of fluid) flow,
 * coupled to a ConservativeDiffuseInterfaceFlowSegregated physics.
 *
 * The transported variable is the liquid volume fraction alpha. Its equation is the scalar
 * transport of the parent with the diffusion and source terms removed and the advection kernel
 * reading lagged_corrected_face_phi, the face flux the Rhie-Chow object of the flow physics
 * froze for the current outer iteration, instead of the Rhie-Chow object itself. The matrix is
 * assembled with upwind face values, and the bounded, sharp solution is produced afterwards by
 * the ConservativeDiffuseInterfaceVOFMULESCorrector this physics creates, which also publishes
 * the alpha_phi_limited and rho_phi face fluxes the flow and energy physics advect with.
 *
 * It also creates the mixture property functors the flow reads, rho_mixture and mu_mixture as
 * alpha-weighted averages of the two phases, and the complementary fraction 1 - alpha. The
 * compression term and the interface normal it needs are the parameters compression_factor and
 * interface_normal_functor; the normal itself is published by the flow physics.
 */
class WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics final
  : public WCNSLinearFVScalarTransportPhysics
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics(const InputParameters & parameters);

protected:
  void addInitialConditions() override;
  void addMaterials() override;
  void addUserObjects() override;

  /// The Van Leer face value of the corrector reads the upwind cell's gradient, which needs that
  /// cell's own neighbours, so two layers
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override { return 2; }

private:
  void addScalarAdvectionKernels() override;
  /// The volume fraction has no diffusion; the compression term lives in the corrector. A
  /// volume source (passive_scalar_source), such as a phase-change rate or a manufactured
  /// forcing, enters the donor solve through the inherited source kernel.
  void addScalarDiffusionKernels() override {}
  /// A volume fraction prescribed at an inlet is the inherited fixed-value condition. A wall
  /// carries no volume fraction condition, so that one stays empty.
  void addScalarWallBC() override {}
  void addScalarOutletBC() override;

  /// An alpha-weighted mixture of two phase properties, optionally with alpha clamped to [0, 1]
  void addMixtureFunctorMaterial(const std::string & object_suffix,
                                 const MooseFunctorName & property_name,
                                 const MooseFunctorName & phase_1_name,
                                 const MooseFunctorName & phase_2_name,
                                 bool limit_phase_fraction);
};
