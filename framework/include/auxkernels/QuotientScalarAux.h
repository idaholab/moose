//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

/**
 * This auxiliary kernel computes its value by dividing "numerator" by
 * "denominator.  For efficiency, it doesn't check the denominator for
 * zero before dividing.
 */
class QuotientScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  QuotientScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _a;
  const VariableValue & _b;
};
