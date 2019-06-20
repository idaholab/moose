#pragma once

#include "THMAction.h"

class THMSetupQuadratureAction;

template <>
InputParameters validParams<THMSetupQuadratureAction>();

/**
 * Sets the quadrature
 */
class THMSetupQuadratureAction : public THMAction
{
public:
  THMSetupQuadratureAction(InputParameters parameters);

  virtual void act() override;
};
