//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

/**
 * Lagrange multiplier "FaceKernel" that is used in conjunction with
 * EqualGradientLagrangeInterface.
 */
class EqualGradientLagrangeMultiplier : public InterfaceKernel
{
public:
  static InputParameters validParams();

  EqualGradientLagrangeMultiplier(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  /// x,y,z component of the gradient to constrain
  const unsigned int _component;

  ///@{ variable to control gradient on the primary side of the interface
  const VariableGradient & _grad_element_value;
  unsigned int _element_jvar;
  ///@}

  /// variable to control gradient on the secondary side of the interface
  unsigned int _neighbor_jvar;

  /// compensate Jacobian fill term from NullKernel
  const Real _jacobian_fill;
};
