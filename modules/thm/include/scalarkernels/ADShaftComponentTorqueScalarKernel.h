#pragma once

#include "ADScalarKernel.h"

class ADShaftConnectableUserObjectInterface;

/**
 * Torque contributed by a component connected to a shaft
 */
class ADShaftComponentTorqueScalarKernel : public ADScalarKernel
{
public:
  ADShaftComponentTorqueScalarKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Shaft connected component user object
  const ADShaftConnectableUserObjectInterface & _shaft_connected_component_uo;

public:
  static InputParameters validParams();
};
