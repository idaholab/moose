//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomMaterial.h"

registerMooseObject("MooseTestApp", RandomMaterial);

InputParameters
RandomMaterial::validParams()
{
  InputParameters params = Material::validParams();
  return params;
}

RandomMaterial::RandomMaterial(const InputParameters & parameters)
  : Material(parameters),
    _rand_real(declareProperty<Real>("rand_real")),
    _rand_long(declareProperty<unsigned long>("rand_long"))
{
  setRandomResetFrequency(EXEC_TIMESTEP_END);
}

void
RandomMaterial::computeQpProperties()
{
  _rand_real[_qp] = getRandomReal();
  _rand_long[_qp] = getRandomLong();
}
