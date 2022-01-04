#pragma once

#include "TotalPowerBase.h"

/**
 * DEPRECATED: Do not use
 */
class PrescribedReactorPower : public TotalPowerBase
{
public:
  PrescribedReactorPower(const InputParameters & parameters);

public:
  static InputParameters validParams();
};
