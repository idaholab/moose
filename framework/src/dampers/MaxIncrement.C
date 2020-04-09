//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxIncrement.h"

registerMooseObject("MooseApp", MaxIncrement);

InputParameters
MaxIncrement::validParams()
{
  InputParameters params = ElementDamper::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "max_increment", "max_increment > 0", "The maximum newton increment for the variable.");
  MooseEnum increment_type("absolute fractional", "absolute");
  params.addParam<MooseEnum>(
      "increment_type",
      increment_type,
      "Type of increment to compare against max_increment. 'absolute': use variable increment. "
      "'fractional': use variable increment divided by the variable value.");

  params.addClassDescription("Limits a variable's update by some max fraction");

  return params;
}

MaxIncrement::MaxIncrement(const InputParameters & parameters)
  : ElementDamper(parameters),
    _max_increment(parameters.get<Real>("max_increment")),
    _increment_type(getParam<MooseEnum>("increment_type").getEnum<IncrementTypeEnum>())
{
}

Real
MaxIncrement::computeQpDamping()
{
  Real inc = std::abs(_u_increment[_qp]);
  if (_increment_type == IncrementTypeEnum::fractional)
    inc /= std::abs(_u[_qp]);

  if (inc > _max_increment)
    return _max_increment / inc;

  return 1.0;
}
