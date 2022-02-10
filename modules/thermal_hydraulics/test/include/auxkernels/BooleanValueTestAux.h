//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Takes a boolean value and converts it into a Real value (0 for false, 1 for true)
 */
class BooleanValueTestAux : public AuxKernel
{
public:
  BooleanValueTestAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const bool & _value;

public:
  static InputParameters validParams();
};
