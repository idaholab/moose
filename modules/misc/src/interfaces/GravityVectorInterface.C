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
#include "MooseUtils.h"
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

    _gravity_magnitude = moose_object->getParam<Real>("gravity_magnitude");

    // Direction is normalized below
    _gravity_direction = moose_object->getParam<RealVectorValue>("gravity_direction");

    // If gravity is zero, direction is the zero vector (which is initial value)
    if (MooseUtils::absoluteFuzzyEqual(_gravity_direction.norm(), 0.0))
    {
      if (!MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0))
        mooseError("If 'gravity_direction' is zero, then 'gravity_magnitude' must also be zero.");
    }
    else
    {
      // If gravity is zero, direction is the zero vector
      if (MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0))
        _gravity_direction = RealVectorValue(0, 0, 0);
      else
        _gravity_direction = _gravity_direction.unit();
    }

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

    // If gravity is zero, direction is the zero vector
    if (MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0))
      _gravity_direction = RealVectorValue(0, 0, 0);
    else
      _gravity_direction = _gravity_vector / _gravity_magnitude;
  }
  else
    moose_object->paramError("gravity_vector",
                             "Either 'gravity_vector' or 'gravity_direction' must be specified.");
}
