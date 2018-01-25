//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ISOTROPICTEMPDEPHARDENING_H
#define ISOTROPICTEMPDEPHARDENING_H

#include "IsotropicPlasticity.h"

class PiecewiseLinear;
class LinearInterpolation;

class IsotropicTempDepHardening;

template <>
InputParameters validParams<IsotropicTempDepHardening>();

class IsotropicTempDepHardening : public IsotropicPlasticity
{
public:
  IsotropicTempDepHardening(const InputParameters & parameters);

protected:
  virtual void computeYieldStress();
  virtual void computeStressInitialize(Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor);

  virtual Real computeHardeningValue(Real scalar);
  virtual Real computeHardeningDerivative(Real scalar);

  void initializeHardeningFunctions();

  MooseSharedPointer<LinearInterpolation> _interp_yield_stress;
  const std::vector<FunctionName> _hardening_functions_names;
  std::vector<PiecewiseLinear *> _hardening_functions;
  std::vector<Real> _hf_temperatures;
  unsigned int _hf_index_lo;
  unsigned int _hf_index_hi;
  Real _hf_fraction;
};

#endif // ISOTROPICTEMPDEPHARDENING_H
