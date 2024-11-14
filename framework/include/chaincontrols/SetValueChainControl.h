//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControl.h"

/**
 * Sets parameter(s) to a control data value.
 */
template <typename T>
class SetValueChainControlTempl : public ChainControl
{
public:
  static InputParameters validParams();

  SetValueChainControlTempl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Value to which to set parameter(s)
  const T & _value;
};

typedef SetValueChainControlTempl<Real> SetRealValueChainControl;
typedef SetValueChainControlTempl<bool> SetBoolValueChainControl;
