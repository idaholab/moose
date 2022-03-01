//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearCombinationFunction.h"

registerMooseObject("MooseApp", LinearCombinationFunction);

InputParameters
LinearCombinationFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<std::vector<FunctionName>>(
      "functions", "This function will return Sum_over_i(w_i * functions_i)");
  params.addRequiredParam<std::vector<Real>>(
      "w", "This function will return Sum_over_i(w_i * functions_i)");
  params.addClassDescription("Returns the linear combination of the functions");
  return params;
}

LinearCombinationFunction::LinearCombinationFunction(const InputParameters & parameters)
  : Function(parameters), FunctionInterface(this)
{
  const auto fname_w = getParam<FunctionName, Real>("functions", "w");

  for (const auto & fw : fname_w)
  {
    if (name() == fw.first)
      paramError("functions", "A LinearCombinationFunction must not reference itself");

    _fw.emplace_back(&getFunctionByName(fw.first), fw.second);
  }
}

Real
LinearCombinationFunction::value(Real t, const Point & p) const
{
  Real val = 0;
  for (const auto & fw : _fw)
    val += fw.first->value(t, p) * fw.second;
  return val;
}

ADReal
LinearCombinationFunction::value(const ADReal & t, const ADPoint & p) const
{
  ADReal val = 0.0;
  for (const auto & fw : _fw)
    val += fw.first->value(t, p) * fw.second;
  return val;
}

RealGradient
LinearCombinationFunction::gradient(Real t, const Point & p) const
{
  RealGradient g;
  for (const auto & fw : _fw)
    g += fw.first->gradient(t, p) * fw.second;
  return g;
}

RealVectorValue
LinearCombinationFunction::vectorValue(Real t, const Point & p) const
{
  RealVectorValue v;
  for (const auto & fw : _fw)
    v += fw.first->vectorValue(t, p) * fw.second;
  return v;
}
