//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Reports the cumulative number of remesh events performed by the [Remeshing] engine.
 */
class RemeshCount : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RemeshCount(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  /// @return the number of remesh events so far, or zero when remeshing is off
  virtual Real getValue() const override;
};
