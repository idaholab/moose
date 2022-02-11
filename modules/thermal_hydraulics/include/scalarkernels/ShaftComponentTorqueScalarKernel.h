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
 * Torque contributed by a component connected to a shaft
 */
class ShaftComponentTorqueScalarKernel : public ScalarKernel
{
public:
  ShaftComponentTorqueScalarKernel(const InputParameters & parameters);

  virtual void reinit() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;

protected:
  /// Shaft connected component user object
  const ShaftConnectableUserObjectInterface & _shaft_connected_component_uo;

public:
  static InputParameters validParams();
};
