#pragma once

#include "Action.h"

class THMSetupQuadratureAction;

template <>
InputParameters validParams<THMSetupQuadratureAction>();

/**
 * Sets the quadrature
 */
class THMSetupQuadratureAction : public Action
{
public:
  THMSetupQuadratureAction(InputParameters parameters);

  virtual void act() override;
};
