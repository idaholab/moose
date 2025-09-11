///* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADPenaltyVelocityContinuity.h"
#include "MooseVariableFE.h"

/**
 * Interface kernel for enforcing continuity of stress and velocity
 */
class ADPenaltyVelocityContinuityNewmarkBeta : public ADPenaltyVelocityContinuity
{
public:
  static InputParameters validParams();

  ADPenaltyVelocityContinuityNewmarkBeta(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue solidVelocity(const unsigned int qp) const override;

  /// The solid velocities from the previous time step. Index is by dimension
  std::vector<const VariableValue *> _solid_velocities_old;

  /// The solid accelerations from the previous time step. Index is by dimension
  std::vector<const VariableValue *> _solid_accelerations_old;

  /// The displacement fields. Index is by dimension
  std::vector<const ADVariableValue *> _displacement_values;

  /// The displacement fields from the previous timestep. Index is by dimension
  std::vector<const VariableValue *> _displacement_values_old;

  /// Newmark-Beta time integration parameters
  const Real _beta;
  const Real _gamma;
};
