/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSHARDENINGCONSTANT_H
#define TENSORMECHANICSHARDENINGCONSTANT_H

#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsHardeningConstant;


template<>
InputParameters validParams<TensorMechanicsHardeningConstant>();

/**
 * No hardening - the parameter assumes the value _val
 * for all internal parameters
 */
class TensorMechanicsHardeningConstant : public TensorMechanicsHardeningModel
{
 public:
  TensorMechanicsHardeningConstant(const std::string & name, InputParameters parameters);

  virtual Real value(const Real & intnl) const;

  virtual Real derivative(const Real & intnl) const;

 private:

  /// The value that the parameter will take
  Real _val;

};

#endif // TENSORMECHANICSHARDENINGCONSTANT_H
