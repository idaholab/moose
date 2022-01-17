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

public:
  static InputParameters validParams();
};
