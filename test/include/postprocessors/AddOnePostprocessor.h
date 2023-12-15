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

/**
 * This PP increase its value by 1 every time timestepSetup() is called
 */
class AddOnePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  AddOnePostprocessor(const InputParameters & params);

  virtual void initialSetup() override { _value = 0; }
  virtual void timestepSetup() override { _value += 1; }
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() const override { return _value; }
  virtual void problemRestoring() override { _value -= 1; }

protected:
  Real _value;
};
