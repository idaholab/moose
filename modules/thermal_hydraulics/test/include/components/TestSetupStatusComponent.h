//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Component used to test setup-status-checking capability
 */
class TestSetupStatusComponent : public Component
{
public:
  TestSetupStatusComponent(const InputParameters & params);

protected:
  virtual void init() override;

public:
  static InputParameters validParams();
};
