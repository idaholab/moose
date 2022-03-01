//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositeFunction.h"

registerMooseObject("MooseApp", CompositeFunction);

InputParameters
CompositeFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addParam<std::vector<FunctionName>>("functions",
                                             "The functions to be multiplied together.");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  params.addClassDescription("Multiplies an arbitrary set of functions together");
  return params;
}

CompositeFunction::CompositeFunction(const InputParameters & parameters)
  : Function(parameters), FunctionInterface(this), _scale_factor(getParam<Real>("scale_factor"))
{

  const std::vector<FunctionName> & names = getParam<std::vector<FunctionName>>("functions");
  const unsigned int len = names.size();
  if (len == 0)
    mooseError("A composite function must reference at least one other function");

  _f.resize(len);

  for (unsigned i = 0; i < len; ++i)
  {
    if (name() == names[i])
      mooseError("A composite function must not reference itself");

    const Function * f = &getFunctionByName(names[i]);
    if (!f)
    {
      std::string msg("Error in composite function ");
      msg += name();
      msg += ".  Function ";
      msg += names[i];
      msg += " referenced but not found.";
      mooseError(msg);
    }
    _f[i] = f;
  }
}

Real
CompositeFunction::value(Real t, const Point & p) const
{
  Real val = _scale_factor;

  for (const auto & func : _f)
    val *= func->value(t, p);

  return val;
}

ADReal
CompositeFunction::value(const ADReal & t, const ADPoint & p) const
{
  ADReal val = _scale_factor;

  for (const auto & func : _f)
    val *= func->value(t, p);

  return val;
}
