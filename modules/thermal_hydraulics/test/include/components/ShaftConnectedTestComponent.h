//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VolumeJunction1Phase.h"
#include "ShaftConnectable.h"

/**
 * Component that shows how to connect a junction-like component to a shaft
 */
class ShaftConnectedTestComponent : public VolumeJunction1Phase, public ShaftConnectable
{
public:
  ShaftConnectedTestComponent(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  const VariableName _jct_var_name;

public:
  static InputParameters validParams();
};
