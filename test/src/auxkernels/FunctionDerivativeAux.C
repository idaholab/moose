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

#include "FunctionDerivativeAux.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionDerivativeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  params.addRequiredParam<unsigned int>(
      "component",
      "What component to take the derivative with respect to. Should be either 1, 2, or 3.");
  return params;
}

FunctionDerivativeAux::FunctionDerivativeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _func(getFunction("function")),
    _component(getParam<unsigned int>("component"))
{
}

Real
FunctionDerivativeAux::computeValue()
{
  if (isNodal())
    return _func.gradient(_t, *_current_node)(_component - 1);
  else
    return _func.gradient(_t, _q_point[_qp])(_component - 1);
}
