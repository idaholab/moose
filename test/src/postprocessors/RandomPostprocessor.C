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

// MOOSE includes
#include "RandomPostprocessor.h"

template <>
InputParameters
validParams<RandomPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  params.addParam<unsigned int>("seed", 0, "Seed the random number generator.");
  params.addParam<unsigned int>("generator", 0, "ID to use for the random number stream.");

  return params;
}

RandomPostprocessor::RandomPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _generator_id(getParam<unsigned int>("generator")),
    _random(declareRestartableData<MooseRandom>("random_pps"))
{
  _random.seed(_generator_id, getParam<unsigned int>("seed"));
}

Real
RandomPostprocessor::getValue()
{
  return _random.rand(_generator_id);
}
