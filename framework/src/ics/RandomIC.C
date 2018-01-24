//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomIC.h"
#include "MooseRandom.h"

#include "libmesh/point.h"

template <>
InputParameters
validParams<RandomIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addParam<Real>("min", 0.0, "Lower bound of the randomly generated values");
  params.addParam<Real>("max", 1.0, "Upper bound of the randomly generated values");
  params.addParam<unsigned int>("seed", 0, "Seed value for the random number generator");
  return params;
}

RandomIC::RandomIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _min(getParam<Real>("min")),
    _max(getParam<Real>("max")),
    _range(_max - _min)
{
  mooseAssert(_range > 0.0, "Min > Max for RandomIC!");
  MooseRandom::seed(getParam<unsigned int>("seed"));
}

Real
RandomIC::value(const Point & /*p*/)
{
  // Random number between 0 and 1
  Real rand_num = MooseRandom::rand();

  // Between 0 and range
  rand_num *= _range;

  // Between min and max
  rand_num += _min;

  return rand_num;
}
