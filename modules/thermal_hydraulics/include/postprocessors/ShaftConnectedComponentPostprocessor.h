#pragma once

#include "GeneralPostprocessor.h"

class ADShaftConnectableUserObjectInterface;

/**
 * Gets torque or moment of inertia for a shaft-connected component.
 */
class ShaftConnectedComponentPostprocessor : public GeneralPostprocessor
{
public:
  ShaftConnectedComponentPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  /// Quantity type
  enum class Quantity
  {
    TORQUE,
    INERTIA
  };
  /// Quantity to get
  const Quantity _quantity;

  /// Shaft-connected component user object
  const ADShaftConnectableUserObjectInterface & _component_uo;

public:
  static InputParameters validParams();
};
