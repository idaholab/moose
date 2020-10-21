//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyTestFunction.h"

#include "MathUtils.h"

registerMooseObject("MooseTestApp", PolyTestFunction);

InputParameters
PolyTestFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<std::vector<Real>>("coefficients",
                                             "Coefficients to use for the polynomial evaluation");
  params.addParam<bool>(
      "derivative", false, "Flag to calculate the derivative of the polynomial function");
  return params;
}

PolyTestFunction::PolyTestFunction(const InputParameters & parameters)
  : Function(parameters),
    _coeffs(getParam<std::vector<Real>>("coefficients")),
    _deriv(getParam<bool>("derivative"))
{
}

Real
PolyTestFunction::value(Real /*t*/, const Point & p) const
{
  return MathUtils::poly(_coeffs, p(0), _deriv);
}
