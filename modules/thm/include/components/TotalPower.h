#pragma once

#include "TotalPowerBase.h"

class TotalPower;

template <>
InputParameters validParams<TotalPower>();

/**
 * Prescribes total power via a user supplied value
 */
class TotalPower : public TotalPowerBase
{
public:
  TotalPower(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// The value of power
  const Real & _power;
};
