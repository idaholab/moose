//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GravityInterface.h"
#include "MooseUtils.h"
#include "Numerics.h"

InputParameters
GravityInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<RealVectorValue>(
      "gravity_vector", THM::default_gravity_vector, "Gravitational acceleration vector [m/s^2]");
  return params;
}

GravityInterface::GravityInterface(const InputParameters & parameters)
  : _gravity_vector(parameters.get<RealVectorValue>("gravity_vector")),
    _gravity_magnitude(_gravity_vector.norm()),
    _gravity_is_zero(MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0)),
    _gravity_direction(_gravity_is_zero ? RealVectorValue(0.0, 0.0, 0.0) : _gravity_vector.unit())
{
}
