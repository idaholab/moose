//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RandomPostprocessor.h"

registerMooseObject("MooseTestApp", RandomPostprocessor);

InputParameters
RandomPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

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
