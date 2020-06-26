#pragma once

#include "VolumeJunctionBase.h"
#include "ShaftConnectable.h"

/**
 * Component that shows how to connect a junction-like component to a shaft
 */
class ShaftConnectedTestComponent : public VolumeJunctionBase, public ShaftConnectable
{
public:
  ShaftConnectedTestComponent(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  const VariableName _jct_var_name;

public:
  static InputParameters validParams();
};
