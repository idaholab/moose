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

#include "LinearCombinationFunction.h"

template <>
InputParameters
validParams<LinearCombinationFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::vector<FunctionName>>(
      "functions", "This function will return Sum_over_i(w_i * functions_i)");
  params.addRequiredParam<std::vector<Real>>(
      "w", "This function will return Sum_over_i(w_i * functions_i)");
  params.addClassDescription("Returns the linear combination of the functions");
  return params;
}

LinearCombinationFunction::LinearCombinationFunction(const InputParameters & parameters)
  : Function(parameters), FunctionInterface(this), _w(getParam<std::vector<Real>>("w"))
{

  const std::vector<FunctionName> & names = getParam<std::vector<FunctionName>>("functions");
  const unsigned int len = names.size();
  if (len != _w.size())
    mooseError(
        "LinearCombinationFunction: The number of functions must equal the number of w values");

  _f.resize(len);
  for (unsigned i = 0; i < len; ++i)
  {
    if (name() == names[i])
      mooseError("A LinearCombinationFunction must not reference itself");
    Function * const f = &getFunctionByName(names[i]);
    if (!f)
      mooseError("LinearCombinationFunction: The function ",
                 names[i],
                 " (referenced by ",
                 name(),
                 ") cannot be found");
    _f[i] = f;
  }
}

Real
LinearCombinationFunction::value(Real t, const Point & p)
{
  Real val = 0;
  for (unsigned i = 0; i < _f.size(); ++i)
    val += _w[i] * _f[i]->value(t, p);
  return val;
}
