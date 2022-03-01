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

class VariableInnerProduct : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  VariableInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the values of second_variable at current quadrature points
  const VariableValue & _v;
};
