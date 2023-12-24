//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundsBase.h"

/**
 * Provides a bound of a variable using its old value.
 */
class VariableOldValueBounds : public BoundsBase
{
public:
  static InputParameters validParams();

  VariableOldValueBounds(const InputParameters & parameters);

protected:
  virtual Real getBound() override;
};
