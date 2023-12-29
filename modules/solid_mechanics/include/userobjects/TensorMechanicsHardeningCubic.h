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
 * Cubic hardening
 * value = _val_0 for p <= _intnl_0
 * value = _val_res for p >= _intnl_limit
 * value = cubic betwen _val_0 at p = _intnl_0, and _val_res at p = _intnl_limit
 * The cubic is smooth, which means nice numerical properties
 */
class TensorMechanicsHardeningCubic : public TensorMechanicsHardeningModel
{
public:
  static InputParameters validParams();

  TensorMechanicsHardeningCubic(const InputParameters & parameters);

  virtual Real value(Real intnl) const override;

  virtual Real derivative(Real intnl) const override;

  virtual std::string modelName() const override;

private:
  /// value is cubic between _val_0 at internal_parameter=_intnl_0, at _val_res at internal_parameter=_intnl_limit
  Real _val_0;

  /// value is cubic between _val_0 at internal_parameter=_intnl_0, at _val_res at internal_parameter=_intnl_limit
  Real _val_res;

  /// value is cubic between _val_0 at internal_parameter=_intnl_0, at _val_res at internal_parameter=_intnl_limit
  Real _intnl_0;

  /// value is cubic between _val_0 at internal_parameter=_intnl_0, at _val_res at internal_parameter=_intnl_limit
  Real _intnl_limit;

  /// convenience parameter for cubic
  Real _half_intnl_limit;

  /// convenience parameter for cubic
  Real _alpha;

  /// convenience parameter for cubic
  Real _beta;
};
