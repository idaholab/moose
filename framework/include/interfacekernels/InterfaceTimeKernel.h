//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// local includes
#include "InterfaceKernel.h"

/**
 * InterfaceTimeKernel is responsible for adding time derivative contributions for physics across
 * interfaces
 */
class InterfaceTimeKernel : public InterfaceKernel
{
public:
  static InputParameters validParams();

  InterfaceTimeKernel(const InputParameters & parameters);

protected:
  /// Compute residuals at quadrature points
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

  /// Compute jacobians at quadrature points
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;

  /// compute off-diagonal jacobian components at quadrature points
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /// Holds the current variable time derivative at the current quadrature point on the face.
  const VariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  const VariableValue & _du_dot_du;

  /// Coupled neighbor variable value time derivative
  const VariableValue & _neighbor_value_dot;

  /// Derivative of _neighbor_value_dot with respect to u
  const VariableValue & _dneighbor_value_dot_du;
};
