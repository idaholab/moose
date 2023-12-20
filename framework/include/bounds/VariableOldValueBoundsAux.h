//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundsAuxBase.h"

/**
 * Provides a bound of a variable using its old value.
 */
class VariableOldValueBoundsAux : public BoundsAuxBase
{
public:
  static InputParameters validParams();

  VariableOldValueBoundsAux(const InputParameters & parameters);

protected:
  virtual Real getBound() override;
};
