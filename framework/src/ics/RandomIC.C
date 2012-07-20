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

#include "RandomIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<RandomIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addParam<Real>("min", 0.0, "Lower bound of the randomly generated values");
  params.addParam<Real>("max", 1.0, "Upper bound of the randomly generated values");
  params.addParam<unsigned int>("seed", 0, "Seed value for the random number generator");
  return params;
}

RandomIC::RandomIC(const std::string & name, InputParameters parameters) :
    InitialCondition(name, parameters),
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
  //Random number between 0 and 1
  Real rand_num = MooseRandom::rand();

  //Between 0 and range
  rand_num *= _range;

  //Between min and max
  rand_num += _min;

  return rand_num;
}
