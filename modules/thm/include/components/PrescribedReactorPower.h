#pragma once

#include "ReactorPower.h"

class PrescribedReactorPower;

template <>
InputParameters validParams<PrescribedReactorPower>();

/**
 * Prescribes reactor power via a user supplied function
 */
class PrescribedReactorPower : public ReactorPower
{
public:
  PrescribedReactorPower(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// The value of power
  const Real & _power;
};
