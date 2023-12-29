//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsHardeningModel.h"

/**
 * Power-Rule Hardening defined by:
 * assuming p = internal_parameter, then value = value_0 * (p / epsilon0 + 1)^{exponent})
 * Notice that if epsilon0 = 0, it will return not a number.
 */
class TensorMechanicsHardeningPowerRule : public TensorMechanicsHardeningModel
{
public:
  static InputParameters validParams();

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
