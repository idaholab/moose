//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralUserObject.h"
#include "MooseVariable.h"
#include "MortarContactUtils.h"

class WeightedGapUserObject;
class WeightedVelocitiesUserObject;

/**
 * Verifies the values and displacement derivatives of mechanical mortar nodal geometry.
 */
class NodalNormalDerivativesTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  NodalNormalDerivativesTest(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;
  void finalize() override {}

private:
  const WeightedGapUserObject & _weighted_gap_uo;
  const WeightedVelocitiesUserObject * const _weighted_velocities_uo;
  const MooseVariable * const _disp_x_var;
  const MooseVariable * const _disp_y_var;
  const MooseVariable * const _disp_z_var;
  const bool _require_distributed_velocity_derivatives;
  const bool _error_after_jacobian_assembly;
  Moose::Mortar::Contact::NodalNormalDerivativeCache _mode_cache;
  bool _saw_residual_mode = false;
  bool _saw_jacobian_mode = false;
  bool _verified_mode_transition = false;
  bool _verified_jacobian_geometry = false;
};
