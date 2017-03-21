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

#include "Distribution.h"
#include "MooseRandom.h"

template <>
InputParameters
validParams<Distribution>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<unsigned int>("seed", 10318691, "Random number generator seed");
  params.registerBase("Distribution");

  return params;
}

Distribution::Distribution(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(parameters, "Distributions"),
    _tid(getParam<THREAD_ID>("_tid")),
    _seed(getParam<unsigned int>("seed")),
    _random(declareRestartableData<MooseRandom>("random"))
{
  setSeed(_seed);
}

Real
Distribution::getRandomNumber()
{
  return inverseCdf(_random.rand(_tid));
}

unsigned int
Distribution::getSeed()
{
  return _seed;
}

void
Distribution::setSeed(unsigned int seed)
{
  _random.seed(_tid, seed);
}
