/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSHARDENINGCUTEXPONENTIAL_H
#define TENSORMECHANICSHARDENINGCUTEXPONENTIAL_H

#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsHardeningCutExponential;

template <>
InputParameters validParams<TensorMechanicsHardeningCutExponential>();

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

#endif // TENSORMECHANICSHARDENINGCUTEXPONENTIAL_H
