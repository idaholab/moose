/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSHARDENINGEXPONENTIAL_H
#define TENSORMECHANICSHARDENINGEXPONENTIAL_H

#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsHardeningExponential;

template <>
InputParameters validParams<TensorMechanicsHardeningExponential>();

/**
 * Exponential hardening
 * The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
 * Note that while this is C-infinity, it produces unphysical results for
 * internal_parameter<0, which can cause numerical problems.
 */
class TensorMechanicsHardeningExponential : public TensorMechanicsHardeningModel
{
public:
  TensorMechanicsHardeningExponential(const InputParameters & parameters);

  virtual Real value(Real intnl) const override;

  virtual Real derivative(Real intnl) const override;

  virtual std::string modelName() const override;

private:
  /// The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
  Real _val_0;

  /// The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
  Real _val_res;

  /// The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
  Real _rate;
};

#endif // TENSORMECHANICSHARDENINGEXPONENTIAL_H
