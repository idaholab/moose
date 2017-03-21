/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TEMPERATUREDEPENDENTHARDENINGSTRESSUPDATE_H
#define TEMPERATUREDEPENDENTHARDENINGSTRESSUPDATE_H

#include "IsotropicPlasticityStressUpdate.h"

class PiecewiseLinear;
class LinearInterpolation;

/**
 * This class inherits from IsotropicPlasticityStressUpdate. It
 * calculates stress as a function of temperature and plastic strain by
 * interpolating hardening functions at different temperatures input by the user.
 */

class TemperatureDependentHardeningStressUpdate : public IsotropicPlasticityStressUpdate
{
public:
  TemperatureDependentHardeningStressUpdate(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(Real effectiveTrialStress) override;

  virtual void computeYieldStress() override;
  virtual Real computeHardeningValue(Real scalar) override;
  virtual Real computeHardeningDerivative(Real scalar) override;

  void initializeHardeningFunctions();

  MooseSharedPointer<LinearInterpolation> _interp_yield_stress;
  const std::vector<FunctionName> _hardening_functions_names;
  std::vector<PiecewiseLinear *> _hardening_functions;
  std::vector<Real> _hf_temperatures;
  unsigned int _hf_index_lo;
  unsigned int _hf_index_hi;
  Real _hf_fraction;
};

template <>
InputParameters validParams<TemperatureDependentHardeningStressUpdate>();

#endif // TEMPERATUREDEPENDENTHARDENINGSTRESSUPDATE_H
