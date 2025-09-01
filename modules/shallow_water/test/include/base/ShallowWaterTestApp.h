//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ShallowWaterApp.h"

class ShallowWaterTestApp : public ShallowWaterApp
{
public:
  static InputParameters validParams();

  ShallowWaterTestApp(const InputParameters & parameters);
  virtual ~ShallowWaterTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
};
