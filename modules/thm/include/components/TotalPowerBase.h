#pragma once

#include "Component.h"

/**
 * Base class for components that provide total power
 */
class TotalPowerBase : public Component
{
public:
  TotalPowerBase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual const VariableName & getPowerVariableName() const { return _power_var_name; }

protected:
  /// The scalar variable holding the value of power
  const VariableName _power_var_name;

public:
  static InputParameters validParams();
};
