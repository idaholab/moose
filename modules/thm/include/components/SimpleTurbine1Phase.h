#pragma once

#include "VolumeJunction1Phase.h"

class SimpleTurbine1Phase;

/**
 * Simple turbine model that extracts prescribed power from the working fluid
 */
class SimpleTurbine1Phase : public VolumeJunction1Phase
{
public:
  SimpleTurbine1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;
  virtual void buildVolumeJunctionUserObject() override;

  /// Flag that specifies if the turbine is operating or not
  const bool & _on;
  /// Turbine power [W]
  const Real & _power;
  /// Variable name that holds power
  VariableName _W_dot_var_name;

public:
  static InputParameters validParams();
};
