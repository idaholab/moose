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
 * Diffusion kernel that adds a per-face jump contribution to the RHS, used to
 * enforce porous baffle pressure jumps.
 */
class LinearFVAnisotropicDiffusionJump : public LinearFVPressureCorrectionDiffusion
{
public:
  static InputParameters validParams();
  LinearFVAnisotropicDiffusionJump(const InputParameters & params);

  Real computeElemMatrixContribution() override;
  Real computeNeighborMatrixContribution() override;
  Real computeElemRightHandSideContribution() override;
  Real computeNeighborRightHandSideContribution() override;

protected:
  /// Compute the baffle transmissibility from the lagged two-term pressure expansion.
  Real computeJumpAwareFluxMatrixContribution();

  const RhieChowMassFlux & _rc_uo;
  const FVReconstructedPressureGradient * const _reconstructed_pressure_gradient_method;
  const bool _use_two_term_pressure_expansion;
};
