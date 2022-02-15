//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectable.h"
#include "Component.h"

InputParameters
ShaftConnectable::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

ShaftConnectable::ShaftConnectable(const Component * const component)
  : _moment_of_inertia_var_name(component->genName(component->name(), "moment_of_inertia")),
    _torque_var_name(component->genName(component->name(), "torque")),
    _user_object_name(component->genName(component->name(), "shaftconnected_uo")),
    _connected_to_shaft(false)
{
}

void
ShaftConnectable::checkShaftConnection(const Component * const component) const
{
  if (!_connected_to_shaft)
    component->logError("This component must be connected to a shaft.");
}

VariableName
ShaftConnectable::getMomentofInertiaVariableName() const
{
  return _moment_of_inertia_var_name;
}

VariableName
ShaftConnectable::getTorqueVariableName() const
{
  return _torque_var_name;
}

UserObjectName
ShaftConnectable::getShaftConnectedUserObjectName() const
{
  return _user_object_name;
}

void
ShaftConnectable::setShaftName(const std::string & name) const
{
  _shaft_name = name;
  _connected_to_shaft = true;
}
