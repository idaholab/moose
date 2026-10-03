//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVPressureCorrectionDiffusion.h"

class RhieChowMassFlux;
class FVReconstructedPressureGradient;

/**
 * Pressure-correction diffusion kernel that enforces modeled jumps on internal baffle faces.
 *
 * The signed pressure jump is added to the right hand side with the same face transmissibility as
 * the pressure-correction operator. An optional two-term pressure expansion derives that
 * transmissibility from lagged reconstructed pressure gradients.
 */
class LinearFVPressureCorrectionDiffusionJump : public LinearFVPressureCorrectionDiffusion
{
public:
  static InputParameters validParams();

  /**
   * Construct the jump-aware pressure-correction diffusion kernel.
   * @param params The input parameters for the kernel
   */
  LinearFVPressureCorrectionDiffusionJump(const InputParameters & params);

  /// Compute the element-row internal-face matrix contribution.
  Real computeElemMatrixContribution() override;

  /// Compute the neighbor-row internal-face matrix contribution.
  Real computeNeighborMatrixContribution() override;

  /// Compute the element-row right hand side, including the element-side pressure jump.
  Real computeElemRightHandSideContribution() override;

  /// Compute the neighbor-row right hand side, including the neighbor-side pressure jump.
  Real computeNeighborRightHandSideContribution() override;

protected:
  /**
   * Compute the pressure-correction transmissibility for the current face.
   *
   * On eligible baffle faces, the two-term option derives the transmissibility from the lagged
   * reconstructed pressure gradients. Otherwise, this returns the base diffusion
   * transmissibility.
   */
  Real computeJumpAwareFluxMatrixContribution();

  /// Rhie-Chow object supplying baffle identification and signed pressure jumps.
  const RhieChowMassFlux & _rc_uo;

  /// Reconstructed pressure-gradient method used by the optional two-term expansion.
  const FVReconstructedPressureGradient * const _reconstructed_pressure_gradient_method;

  /// Whether to derive baffle transmissibility from the lagged two-term pressure expansion.
  const bool _use_two_term_pressure_expansion;
};
