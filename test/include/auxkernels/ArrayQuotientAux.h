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

class ArrayQuotientAux : public ArrayAuxKernel
{
public:
  static InputParameters validParams();

  ArrayQuotientAux(const InputParameters & parameters);

protected:
  virtual RealEigenVector computeValue() override;

  const ArrayVariableValue & _numerator;
  const ArrayVariableValue & _denominator;
};
