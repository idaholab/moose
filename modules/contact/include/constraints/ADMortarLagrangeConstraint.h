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

/**
 * This class enforces mortar constraints on lower dimensional domains, skipping interior nodes.
 * This class is intended to provide an alternative to imposing mortar constraints for finite
 * elements with nodal variables.
 */
class ADMortarLagrangeConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ADMortarLagrangeConstraint(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  using ADMortarConstraint::computeResidual;
  /**
   * compute the residual for the specified element type
   */
  void computeResidual(Moose::MortarType mortar_type) override;

  using ADMortarConstraint::computeJacobian;
  /**
   * compute the Jacobian for the specified element type
   */
  void computeJacobian(Moose::MortarType mortar_type) override;

  /// Nodal map from secondary interior parent to lower dimensional domain
  std::map<unsigned int, unsigned int> _secondary_ip_lowerd_map;

  /// Nodal map from primary interior parent to lower dimensional domain
  std::map<unsigned int, unsigned int> _primary_ip_lowerd_map;

  /// Threshold to discard derivatives and shrink the AD object.
  /// This may help for multiphysics, three-dimensional mortar problems where,
  /// otherwise, the size of the AD derivative container would be prohibitive.
  const Real _ad_derivative_threshold;

private:
  /// Whether to apply derivative trimming. Automatically disabled for the variable
  /// condensation preconditioner (VCP) since the removal of lower-d-related dofs appear
  /// to be causing misbehavior when the no-adaptivity option is employed.
  bool _apply_derivative_threshold;
};
