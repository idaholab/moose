#pragma once

#include "ScalarKernel.h"

class ShaftConnectableUserObjectInterface;

/**
 * Torque contributed by a component connected to a shaft
 */
class ShaftComponentTorqueScalarKernel : public ScalarKernel
{
public:
  ShaftComponentTorqueScalarKernel(const InputParameters & parameters);

  virtual void reinit() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;

protected:
  /// Shaft connected component user object
  const ShaftConnectableUserObjectInterface & _shaft_connected_component_uo;

public:
  static InputParameters validParams();
};
