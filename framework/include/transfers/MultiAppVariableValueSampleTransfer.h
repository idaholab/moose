//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiAppTransfer.h"

/**
 * Samples a variable's value in the parent application domain at the point where
 * the MultiApp (for each child app) is. Copies that value into a field variable in the MultiApp.
 */
class MultiAppVariableValueSampleTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppVariableValueSampleTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  /// Variable to sample in the parent application
  AuxVariableName _to_var_name;

  /// Variable in the MultiApp to fill with the sampled value
  VariableName _from_var_name;
};
