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
 * Computes the normal contact mortar constraints for dynamic simulations.
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

  virtual void computeQpIProperties() override;

  virtual void residualSetup() override;

  void timestepSetup() override;

  virtual void post() override;

  virtual void
  incorrectEdgeDroppingPost(const std::unordered_set<const Node *> & inactive_lm_nodes) override;

  /// A small threshold gap value to consider that a node needs a "persistency" constraint
  const Real _capture_tolerance;

  const ADVariableValue & _secondary_x_dot;
  const ADVariableValue & _primary_x_dot;
  const ADVariableValue & _secondary_y_dot;
  const ADVariableValue & _primary_y_dot;

  const ADVariableValue * _secondary_z_dot;
  const ADVariableValue * _primary_z_dot;

  /// A map from dof-object to the old weighted gap
  std::unordered_map<const DofObject *, ADReal> _dof_to_old_weighted_gap;

  /// Vector for computation of weighted gap velocity to fulfill "persistency" condition
  ADRealVectorValue _qp_gap_nodal_dynamics;

  /// A map from node to weighted gap velocity times _dt
  std::unordered_map<const DofObject *, ADReal> _dof_to_weighted_gap_dynamics;
};
