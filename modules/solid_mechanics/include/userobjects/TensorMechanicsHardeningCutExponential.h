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
 * CutExponential hardening
 * The value = _val_res + (val_0 - val_res)*exp(-rate*(internal_parameter - _intnl_0)), for
 * internal_parameter >= _intnl_0, otherwise value = _val_0
 * Note that while this is not smooth at internal_parameter = _intnl_0,
 * which can produce bad numerical problems.
 */
class TensorMechanicsHardeningCutExponential : public TensorMechanicsHardeningModel
{
public:
  static InputParameters validParams();

  TensorMechanicsHardeningCutExponential(const InputParameters & parameters);

  virtual Real value(Real intnl) const override;

  virtual Real derivative(Real intnl) const override;

  virtual std::string modelName() const override;

private:
  /// The value = _val_res + (val_0 - val_res)*exp(-rate*(internal_parameter - _intnl_0)), for internal_parameter >= _intnl_0, otherwise value = _val_0
  Real _val_0;

  /// The value = _val_res + (val_0 - val_res)*exp(-rate*(internal_parameter - _intnl_0)), for internal_parameter >= _intnl_0, otherwise value = _val_0
  Real _val_res;

  /// The value = _val_res + (val_0 - val_res)*exp(-rate*(internal_parameter - _intnl_0)), for internal_parameter >= _intnl_0, otherwise value = _val_0
  Real _intnl_0;

  /// The value = _val_res + (val_0 - val_res)*exp(-rate*(internal_parameter - _intnl_0)), for internal_parameter >= _intnl_0, otherwise value = _val_0
  Real _rate;
};
