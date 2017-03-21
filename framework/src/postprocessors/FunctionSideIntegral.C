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

#include "FunctionSideIntegral.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionSideIntegral>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addParam<FunctionName>(
      "function",
      1.0,
      "This postprocessor will return the integral of this function over the boundary");
  return params;
}

FunctionSideIntegral::FunctionSideIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters), _func(getFunction("function"))
{
}

void
FunctionSideIntegral::threadJoin(const UserObject & y)
{
  const FunctionSideIntegral & pps = static_cast<const FunctionSideIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
FunctionSideIntegral::computeQpIntegral()
{
  return _func.value(_t, _q_point[_qp]);
}
