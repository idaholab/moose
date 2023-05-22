//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"

class ShaftConnectableUserObjectInterface;

/**
 * Time derivative for angular speed of shaft
 */
class ShaftTimeDerivativeScalarKernel : public ScalarKernel
{
public:
  ShaftTimeDerivativeScalarKernel(const InputParameters & parameters);

  virtual void reinit() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;

protected:
  virtual Real computeQpResidual() override { mooseError("unused"); }
  virtual Real computeQpJacobian() override { mooseError("unused"); }

  /// Time derivative of u
  const VariableValue & _u_dot;
  /// Derivative of u_dot wrt u
  const VariableValue & _du_dot_du;
  /// List of names of shaft connected user objects
  const std::vector<UserObjectName> & _uo_names;
  /// Number of shaft connected user objects
  unsigned int _n_components;
  /// List of shaft connected user objects
  std::vector<const ShaftConnectableUserObjectInterface *> _shaft_connected_uos;

public:
  static InputParameters validParams();
};
