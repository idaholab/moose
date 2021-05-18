//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

#include <unordered_map>

/**
 * Computes the weighted gap that will later be used to enforce the
 * zero-penetration mechanical contact conditions
 */
class ComputeWeightedGapLMMechanicalContact : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ComputeWeightedGapLMMechanicalContact(const InputParameters & parameters);
  using ADMortarConstraint::computeResidual;
  void computeResidual(Moose::MortarType mortar_type) override;
  using ADMortarConstraint::computeJacobian;
  void computeJacobian(Moose::MortarType mortar_type) override;
  void residualSetup() override;
  void jacobianSetup() override;
  void post() override;

protected:
  ADReal computeQpResidual(Moose::MortarType mortar_type) final;
  void computeQpProperties();
  void computeQpIProperties();
  void onNode(const Node * node);

  const ADVariableValue & _secondary_disp_x;
  const ADVariableValue & _primary_disp_x;
  const ADVariableValue & _secondary_disp_y;
  const ADVariableValue & _primary_disp_y;

  /// Whether this object is operating on the displaced mesh
  const bool _displaced;

  /// The normal index. This is _qp if we are interpolating the nodal normals, else it is _i
  const unsigned int & _normal_index;

  const Real _c;
  ADReal _qp_gap;
  std::unordered_map<const Node *, ADReal> _node_to_weighted_gap;
  const ADReal * _weighted_gap_ptr = nullptr;
};
