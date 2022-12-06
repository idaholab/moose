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
#include "THMAppInterface.h"
#include "ControlData.h"

/**
 * Copies a boolean control data value.
 */
class BoolControlDataValuePostprocessor : public GeneralPostprocessor, public THMAppInterface
{
public:
  BoolControlDataValuePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual Real getValue() override;
  virtual void execute() override;

protected:
  /// The boolean value of the control data
  const ControlData<bool> * const _control_data_value;

public:
  static InputParameters validParams();
};
