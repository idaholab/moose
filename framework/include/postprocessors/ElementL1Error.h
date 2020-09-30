//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class Function;

/**
 * Computes L1 error between an elemental field variable and an analytical function.
 */
class ElementL1Error : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  ElementL1Error(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  const Function & _func;
};
