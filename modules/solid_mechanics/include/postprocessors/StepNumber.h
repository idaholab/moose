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
#include "StepUserObject.h"

/**
 * Given the current time, outputs the current analysis step number
 */
class StepNumber : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  StepNumber(const InputParameters & parameters);

  virtual void execute() override {};
  virtual void initialize() override {};
  virtual PostprocessorValue getValue() const override;

private:
  /// The nearest node number UserObject that does all the work
  const StepUserObject & _step_uo;
  const bool _use_one_based_indexing;
};
