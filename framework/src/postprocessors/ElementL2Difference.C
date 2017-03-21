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

#include "ElementL2Difference.h"
#include "Function.h"

template <>
InputParameters
validParams<ElementL2Difference>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addRequiredCoupledVar("other_variable", "The variable to compare to");
  return params;
}

ElementL2Difference::ElementL2Difference(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _other_var(coupledValue("other_variable"))
{
}

Real
ElementL2Difference::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementL2Difference::computeQpIntegral()
{
  Real diff = _u[_qp] - _other_var[_qp];
  return diff * diff;
}
