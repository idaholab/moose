/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSHARDENINGPOWERRULE_H
#define TENSORMECHANICSHARDENINGPOWERRULE_H

#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsHardeningPowerRule;

template <>
InputParameters validParams<TensorMechanicsHardeningPowerRule>();

/**
 * Power-Rule Hardening defined by:
 * assuming p = internal_parameter, then value = value_0 * (p / epsilon0 + 1)^{exponent})
 * Notice that if epsilon0 = 0, it will return not a number.
 */
class TensorMechanicsHardeningPowerRule : public TensorMechanicsHardeningModel
{
public:
  TensorMechanicsHardeningPowerRule(const InputParameters & parameters);

  virtual Real value(Real intnl) const override;

  virtual Real derivative(Real intnl) const override;

  virtual std::string modelName() const override;

private:
  /// The value = value_0 * (p / epsilon0 + 1)^{exponent})
  const Real _value_0;

  /// The value = value_0 * (p / epsilon0 + 1)^{exponent})
  const Real _epsilon0;

  /// The value = value_0 * (p / epsilon0 + 1)^{exponent})
  const Real _exponent;
};

#endif // TENSORMECHANICSHARDENINGPOWERRULE_H
