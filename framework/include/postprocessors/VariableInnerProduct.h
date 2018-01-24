//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VARIABLEINNERPRODUCT_H
#define VARIABLEINNERPRODUCT_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class VariableInnerProduct;

template <>
InputParameters validParams<VariableInnerProduct>();

class VariableInnerProduct : public ElementIntegralVariablePostprocessor
{
public:
  VariableInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the values of second_variable at current quadrature points
  const VariableValue & _v;
};

#endif // VariableInnerProduct_H
