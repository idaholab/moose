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
 * Gaussian hardening
 * The value = _val_res + (val_0 - val_res)*exp(-0.5*rate*(p - intnl_0)^2) for p>intnl_0.  Here p =
 * internal parameter,
 * and value = _val_0 for p<=intnl_0
 * This has nice numerical properties because it is C-infinity continuous
 */
class TensorMechanicsHardeningGaussian : public TensorMechanicsHardeningModel
{
public:
  static InputParameters validParams();

  TensorMechanicsHardeningGaussian(const InputParameters & parameters);

  virtual Real value(Real intnl) const override;

  virtual Real derivative(Real intnl) const override;

  virtual std::string modelName() const override;

private:
  /// The value = _val_res + (val_0 - val_res)*exp(-0.5*rate*(p - intnl_0)^2) for p>intnl_0.  Here p = internal parameter
  Real _val_0;

  /// The value = _val_res + (val_0 - val_res)*exp(-0.5*rate*(p - intnl_0)^2) for p>intnl_0.  Here p = internal parameter
  Real _val_res;

  /// The value = _val_res + (val_0 - val_res)*exp(-0.5*rate*(p - intnl_0)^2) for p>intnl_0.  Here p = internal parameter
  Real _intnl_0;

  /// The value = _val_res + (val_0 - val_res)*exp(-0.5*rate*(p - intnl_0)^2) for p>intnl_0.  Here p = internal parameter
  Real _rate;
};
