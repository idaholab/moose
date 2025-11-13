//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GravityVectorInterface.h"
#include "PhysicalConstants.h"
#include "MooseObject.h"

InputParameters
GravityVectorInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("gravity_magnitude",
                        PhysicalConstants::acceleration_of_gravity,
                        "Magnitude of the gravitational acceleration [m/s^2]");
  params.addParam<RealVectorValue>(
      "gravity_direction",
      "Direction of gravitational acceleration. This will be normalized and multiplied by "
      "'gravity_magnitude'. This parameter is mutually exclusive with 'gravity_vector'.");
  params.addParam<RealVectorValue>("gravity_vector",
                                   "Gravitational acceleration vector [m/s^2]. This parameter is "
                                   "mutually exclusive with 'gravity_direction'.");
  return params;
}

GravityVectorInterface::GravityVectorInterface(const MooseObject * moose_object)
{
  if (moose_object->isParamValid("gravity_direction"))
  {
    if (moose_object->isParamValid("gravity_vector"))
      moose_object->paramError(
          "gravity_direction",
          "The parameters 'gravity_vector' and 'gravity_direction' are mutually exclusive.");

    _gravity_direction = moose_object->getParam<RealVectorValue>("gravity_direction").unit();
    _gravity_magnitude = moose_object->getParam<Real>("gravity_magnitude");
    _gravity_vector = _gravity_magnitude * _gravity_direction;
  }
  else if (moose_object->isParamValid("gravity_vector"))
  {
    if (moose_object->isParamSetByUser("gravity_magnitude"))
      moose_object->paramError(
          "gravity_magnitude",
          "The parameters 'gravity_vector' and 'gravity_magnitude' are mutually exclusive.");

    _gravity_vector = moose_object->getParam<RealVectorValue>("gravity_vector");
    _gravity_magnitude = _gravity_vector.norm();
    _gravity_direction = _gravity_vector / _gravity_magnitude;
  }
  else
    moose_object->paramError("gravity_vector",
                             "Either 'gravity_vector' or 'gravity_direction' must be specified.");
}
