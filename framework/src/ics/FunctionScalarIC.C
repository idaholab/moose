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

#include "FunctionScalarIC.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionScalarIC>()
{
  InputParameters params = validParams<ScalarInitialCondition>();
  params.addRequiredParam<std::vector<FunctionName>>("function", "The initial condition function.");
  return params;
}

FunctionScalarIC::FunctionScalarIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters), _ncomp(_var.order())
{
  std::vector<FunctionName> funcs = getParam<std::vector<FunctionName>>("function");
  if (funcs.size() != _ncomp)
    mooseError("number of functions must be equal to the scalar variable order");

  for (const auto & func_name : funcs)
    _func.push_back(&getFunctionByName(func_name));
}

Real
FunctionScalarIC::value()
{
  return _func[_i]->value(_t, Point(0, 0, 0));
}
