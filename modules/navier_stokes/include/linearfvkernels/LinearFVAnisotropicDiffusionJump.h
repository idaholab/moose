//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAnisotropicDiffusion.h"

class RhieChowMassFlux;

/**
 * Diffusion kernel that adds a per-face jump contribution to the RHS, used to
 * enforce porous baffle pressure jumps.
 */
class LinearFVAnisotropicDiffusionJump : public LinearFVAnisotropicDiffusion
{
public:
  static InputParameters validParams();
  LinearFVAnisotropicDiffusionJump(const InputParameters & params);

  virtual Real computeElemRightHandSideContribution() override;
  virtual Real computeNeighborRightHandSideContribution() override;

protected:
  const RhieChowMassFlux & _rc_uo;
  const bool _debug_baffle_jump;
};
