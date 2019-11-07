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
 * Test Postprocessor uses old values from a VectorPostprocessor
 */
class UseOldVectorPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  UseOldVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

private:
  /// The reference to the _current_ value of a coupled VectorPostprocessor
  const VectorPostprocessorValue & _vec;

  /// The reference to the _old_ value of a coupled VectorPostprocessor
  const VectorPostprocessorValue & _vec_old;

  /// Restartable reference to the _current value
  Real & _value;

  /// The old value (from the old vector, not stateful)
  Real _old_value;
};
