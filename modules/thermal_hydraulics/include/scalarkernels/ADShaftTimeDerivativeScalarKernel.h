#pragma once

#include "ADScalarTimeDerivative.h"

class ADShaftConnectableUserObjectInterface;

/**
 * Time derivative for angular speed of shaft
 */
class ADShaftTimeDerivativeScalarKernel : public ADScalarTimeDerivative
{
public:
  ADShaftTimeDerivativeScalarKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// List of names of shaft connected user objects
  const std::vector<UserObjectName> & _uo_names;
  /// Number of shaft connected user objects
  unsigned int _n_components;
  /// List of shaft connected user objects
  std::vector<const ADShaftConnectableUserObjectInterface *> _shaft_connected_uos;

public:
  static InputParameters validParams();
};
