//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "XFEMMovingInterfaceVelocityBase.h"

class XFEMPhaseTransitionMovingInterfaceVelocity : public XFEMMovingInterfaceVelocityBase
{
public:
  static InputParameters validParams();

  XFEMPhaseTransitionMovingInterfaceVelocity(const InputParameters & parameters);
  virtual ~XFEMPhaseTransitionMovingInterfaceVelocity() {}

  virtual Real computeMovingInterfaceVelocity(dof_id_type point_id,
                                              RealVectorValue normal) const override;

protected:
  /// Diffusivity in the positive level set region
  Real _diffusivity_at_positive_level_set;

  /// Diffusivity in the negative level set region
  Real _diffusivity_at_negative_level_set;

  /// Jump of the equilibrium concentrations at phase boundary
  Real _equilibrium_concentration_jump;
};
