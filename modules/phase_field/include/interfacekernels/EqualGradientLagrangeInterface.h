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
 * InterfaceKernel to enforce a Lagrange-Multiplier based componentwise
 * continuity of a variable gradient.
 */
class EqualGradientLagrangeInterface : public InterfaceKernel
{
public:
  static InputParameters validParams();

  EqualGradientLagrangeInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  const unsigned int _component;

  /// Lagrange multiplier
  const VariableValue & _lambda;

  const unsigned int _lambda_jvar;
};
