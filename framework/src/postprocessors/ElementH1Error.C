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

#include "ElementH1Error.h"
#include "Function.h"

template<>
InputParameters validParams<ElementH1Error>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}

ElementH1Error::ElementH1Error(const std::string & name, InputParameters parameters) :
    ElementIntegralVariablePostprocessor(name, parameters),
    FunctionInterface(parameters),
    _func(getFunction("function"))
{
}

Real
ElementH1Error::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementH1Error::computeQpIntegral()
{
  RealGradient graddiff = _grad_u[_qp]-_func.gradient(_t, _q_point[_qp]);
  Real         funcdiff = _u[_qp]-_func.value(_t, _q_point[_qp]);

  return graddiff*graddiff + funcdiff*funcdiff;
}
