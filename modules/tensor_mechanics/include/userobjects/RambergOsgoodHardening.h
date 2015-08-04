/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RAMBERGOSGOODHARDENING_H
#define RAMBERGOSGOODHARDENING_H

#include "TensorMechanicsHardeningModel.h"

/**
 * This class computes the Flow stress based on Ramberg-Osgood Hardening Model
 */

class RambergOsgoodHardening;

template<>
InputParameters validParams<RambergOsgoodHardening>();

class RambergOsgoodHardening : public TensorMechanicsHardeningModel
{
public:
  RambergOsgoodHardening(const InputParameters & parameters);

  virtual Real value(const Real & intnl) const;
  virtual Real derivative(const Real & intnl) const;

protected:
  ///Yield stress (sigma_0)
  Real _yield_stress;
  ///Hardening exponent (m)
  Real _hardening_exponent;
  ///Reference plastic strain
  Real _ref_plastic_strain;

private:
};

#endif //RAMBERGOSGOODHARDENING_H
