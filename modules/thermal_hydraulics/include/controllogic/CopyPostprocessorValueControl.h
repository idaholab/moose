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
 * This control takes a postprocessor and copies its value into a control data value
 */
class CopyPostprocessorValueControl : public THMControl
{
public:
  CopyPostprocessorValueControl(const InputParameters & parameters);

  virtual void execute();

protected:
  Real & _value;
  const Real & _pps_value;

public:
  static InputParameters validParams();
};
