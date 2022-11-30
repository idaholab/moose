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

class ArrayVarReductionAux : public AuxKernel
{
public:
  static InputParameters validParams();
  ArrayVarReductionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// variable for storing the array variable that needs to be processed
  const ArrayVariableValue & _array_variable;

  /// The type of reduction operation performed on the array variable
  const MooseEnum _value_type;
};
