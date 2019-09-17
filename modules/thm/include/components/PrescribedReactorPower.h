#pragma once

#include "TotalPowerBase.h"

class PrescribedReactorPower;

template <>
InputParameters validParams<PrescribedReactorPower>();

/**
 * DEPRECATED: Do not use
 */
class PrescribedReactorPower : public TotalPowerBase
{
public:
  PrescribedReactorPower(const InputParameters & parameters);
};
