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

#include "ElementW1pError.h"
#include "Function.h"

template <>
InputParameters
validParams<ElementW1pError>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1", "The exponent used in the norm.");
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}

ElementW1pError::ElementW1pError(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _p(getParam<Real>("p")),
    _func(getFunction("function"))
{
}

Real
ElementW1pError::getValue()
{
  return std::pow(ElementIntegralPostprocessor::getValue(), 1. / _p);
}

Real
ElementW1pError::computeQpIntegral()
{
  RealGradient graddiff = _grad_u[_qp] - _func.gradient(_t, _q_point[_qp]);
  Real funcdiff = _u[_qp] - _func.value(_t, _q_point[_qp]);

  // Raise the absolute function value difference to the pth power
  Real val = std::pow(std::abs(funcdiff), _p);

  // Add all of the absolute gradient component differences to the pth power
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    val += std::pow(std::abs(graddiff(i)), _p);

  return val;
}
