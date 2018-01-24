//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxIncrement.h"

template <>
InputParameters
validParams<MaxIncrement>()
{
  InputParameters params = validParams<ElementDamper>();
  params.addRequiredParam<Real>("max_increment", "The maximum newton increment for the variable.");
  return params;
}

MaxIncrement::MaxIncrement(const InputParameters & parameters)
  : ElementDamper(parameters), _max_increment(parameters.get<Real>("max_increment"))
{
}

Real
MaxIncrement::computeQpDamping()
{

  if (std::abs(_u_increment[_qp]) > _max_increment)
  {
    return std::abs(_max_increment / _u_increment[_qp]);
  }

  return 1.0;
}
