//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeWeightedGapLMMechanicalContact.h"

#include <unordered_map>

/**
 * Computes the weighted gap that will later be used to enforce the
 * zero-penetration mechanical contact conditions
 */
class ComputeDynamicWeightedGapLMMechanicalContact : public ComputeWeightedGapLMMechanicalContact
{
public:
  static InputParameters validParams();

  ComputeDynamicWeightedGapLMMechanicalContact(const InputParameters & parameters);

protected:
  /**
   * Computes properties that are functions only of the current quadrature point (\p _qp), e.g.
   * indepedent of shape functions
   */
  virtual void computeQpProperties() override;

  void timestepSetup() override;

  const VariableValue & _secondary_x_old;
  const VariableValue & _primary_x_old;
  const VariableValue & _secondary_y_old;
  const VariableValue & _primary_y_old;

  const ADVariableValue & _secondary_x_dot;
  const ADVariableValue & _primary_x_dot;
  const ADVariableValue & _secondary_y_dot;
  const ADVariableValue & _primary_y_dot;

  const ADVariableValue & _secondary_x_dotdot;
  const ADVariableValue & _primary_x_dotdot;
  const ADVariableValue & _secondary_y_dotdot;
  const ADVariableValue & _primary_y_dotdot;

  const ADVariableValue * _secondary_z_dot;
  const ADVariableValue * _primary_z_dot;
  const ADVariableValue * _secondary_z_dotdot;
  const ADVariableValue * _primary_z_dotdot;

  const bool _has_beta;
  const Real _beta;

  /// A map from dof-object to the old weighted gap
  std::unordered_map<const DofObject *, ADReal> _dof_to_old_weighted_gap;
};
