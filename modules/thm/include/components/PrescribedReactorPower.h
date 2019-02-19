#ifndef PRESCRIBEDREACTORPOWER_H
#define PRESCRIBEDREACTORPOWER_H

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
  /// The function describing the power in time
  const FunctionName & _power_fn;
  /// True if the power was specified as a constant
  bool _const_power;
  /// The value of power when specified as a constant
  Real _power;
};

#endif /* PRESCRIBEDREACTORPOWER_H */
