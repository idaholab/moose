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
 * A class for storing a constant value.
 */
class ConstantPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ConstantPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override;

protected:
  /// value held by postprocessor
  const Real & _value;
};
