//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class ADShaftConnectableUserObjectInterface;

/**
 * Gets torque or moment of inertia for a shaft-connected component.
 */
class ShaftConnectedComponentPostprocessor : public GeneralPostprocessor
{
public:
  ShaftConnectedComponentPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() const override;

protected:
  /// Quantity type
  enum class Quantity
  {
    TORQUE,
    INERTIA
  };
  /// Quantity to get
  const Quantity _quantity;

  /// Shaft-connected component user object
  const ADShaftConnectableUserObjectInterface & _component_uo;

public:
  static InputParameters validParams();
};
