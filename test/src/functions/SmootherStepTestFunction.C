//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmootherStepTestFunction.h"

#include "MathUtils.h"

registerMooseObject("MooseTestApp", SmootherStepTestFunction);

InputParameters
SmootherStepTestFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addParam<bool>(
      "derivative", false, "Flag to calculate the derivative of the polynomial function");
  return params;
}

SmootherStepTestFunction::SmootherStepTestFunction(const InputParameters & parameters)
  : Function(parameters), _derivative(getParam<bool>("derivative"))
{
}

Real
SmootherStepTestFunction::value(Real /*t*/, const Point & p) const
{
  return MathUtils::smootherStep<Real>(p(0), 0.2, 0.8, _derivative);
}
