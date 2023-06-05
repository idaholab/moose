//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMControl.h"

/**
 * Control object that reads a Real value computed by the control logic system and sets it into a
 * specified MOOSE object parameter(s)
 */
class SetRealValueControl : public THMControl
{
public:
  SetRealValueControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// The value that is written into the MOOSE object's input parameter
  const Real & _value;

public:
  static InputParameters validParams();
};
