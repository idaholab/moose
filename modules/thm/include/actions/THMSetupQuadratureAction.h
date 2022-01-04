#pragma once

#include "Action.h"

/**
 * Sets the quadrature
 */
class THMSetupQuadratureAction : public Action
{
public:
  THMSetupQuadratureAction(InputParameters parameters);

  virtual void act() override;

public:
  static InputParameters validParams();
};
