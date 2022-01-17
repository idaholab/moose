#pragma once

#include "Component.h"
#include "ShaftConnectable.h"

/**
 * Motor to drive a shaft component
 */
class ShaftConnectedMotor : public Component, public ShaftConnectable
{
public:
  ShaftConnectedMotor(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// Torque
  const Real & _torque;
  /// Moment of intertia
  const Real & _inertia;

public:
  static InputParameters validParams();
};
