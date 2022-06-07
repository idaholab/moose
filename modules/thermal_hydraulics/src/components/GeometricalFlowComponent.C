//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricalFlowComponent.h"
#include "FluidProperties.h"

InputParameters
GeometricalFlowComponent::validParams()
{
  InputParameters params = Component1D::validParams();
  params += GravityInterface::validParams();

  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object");

  return params;
}

GeometricalFlowComponent::GeometricalFlowComponent(const InputParameters & parameters)
  : Component1D(parameters),
    GravityInterface(parameters),

    _gravity_angle(MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0)
                       ? 0.0
                       : std::acos(_dir * _gravity_vector / (_dir.norm() * _gravity_magnitude)) *
                             180 / M_PI),
    _fp_name(getParam<UserObjectName>("fp"))
{
}
