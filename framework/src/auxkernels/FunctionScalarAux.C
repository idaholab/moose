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

#include "FunctionScalarAux.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionScalarAux>()
{
  InputParameters params = validParams<AuxScalarKernel>();
  params.addRequiredParam<std::vector<FunctionName>>(
      "function", "The functions to set the scalar variable components.");

  return params;
}

FunctionScalarAux::FunctionScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters)
{
  std::vector<FunctionName> funcs = getParam<std::vector<FunctionName>>("function");
  if (funcs.size() != _var.order())
    mooseError("number of functions is not equal to the number of scalar variable components");

  for (const auto & func : funcs)
    _functions.push_back(&getFunctionByName(func));
}

Real
FunctionScalarAux::computeValue()
{
  return _functions[_i]->value(_t, Point(0, 0, 0));
}
