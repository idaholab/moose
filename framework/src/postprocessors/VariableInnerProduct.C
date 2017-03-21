/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "VariableInnerProduct.h"

template <>
InputParameters
validParams<VariableInnerProduct>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addRequiredCoupledVar(
      "second_variable",
      "The name of the second variable in the inner product (variable, second_variable)");
  return params;
}

VariableInnerProduct::VariableInnerProduct(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _v(coupledValue("second_variable"))
{
}

Real
VariableInnerProduct::computeQpIntegral()
{
  return _u[_qp] * _v[_qp];
}
