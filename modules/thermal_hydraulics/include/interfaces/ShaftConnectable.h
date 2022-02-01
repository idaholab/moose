//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

class Component;

/**
 * Interface class for components that connect to a shaft
 */
class ShaftConnectable
{
public:
  ShaftConnectable(const Component * const component);

  virtual void checkShaftConnection(const Component * const component) const;
  virtual VariableName getMomentofInertiaVariableName() const;
  virtual VariableName getTorqueVariableName() const;
  virtual UserObjectName getShaftConnectedUserObjectName() const;
  virtual void setShaftName(const std::string & name) const;

protected:
  /// Moment of inertia variable name
  VariableName _moment_of_inertia_var_name;
  /// Torque of variable name
  VariableName _torque_var_name;
  /// Shaft-connectable user object name
  UserObjectName _user_object_name;
  /// Name of the shaft component
  mutable std::string _shaft_name;
  /// Flag indicating that a shaft has this component connected
  mutable bool _connected_to_shaft;

public:
  static InputParameters validParams();
};
