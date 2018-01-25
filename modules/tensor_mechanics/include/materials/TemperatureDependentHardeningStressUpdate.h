//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TEMPERATUREDEPENDENTHARDENINGSTRESSUPDATE_H
#define TEMPERATUREDEPENDENTHARDENINGSTRESSUPDATE_H

#include "IsotropicPlasticityStressUpdate.h"

class PiecewiseLinear;
class LinearInterpolation;

class TemperatureDependentHardeningStressUpdate;

template <>
InputParameters validParams<TemperatureDependentHardeningStressUpdate>();

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
  virtual void computeStressInitialize(const Real effectiveTrialStress,
                                       const RankFourTensor & elasticity_tensor) override;

  virtual void computeYieldStress(const RankFourTensor & elasticity_tensor) override;
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
