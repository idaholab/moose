/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ISOTROPICTEMPDEPHARDENING_H
#define ISOTROPICTEMPDEPHARDENING_H

#include "IsotropicPlasticity.h"

class PiecewiseLinear;
class LinearInterpolation;

class IsotropicTempDepHardening : public IsotropicPlasticity
{
public:
  IsotropicTempDepHardening(const InputParameters & parameters);

protected:
  virtual void computeYieldStress(unsigned qp);
  virtual void computeStressInitialize(unsigned qp,
                                       Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor);

  virtual Real computeHardeningValue(unsigned qp, Real scalar);
  virtual Real computeHardeningDerivative(unsigned qp, Real scalar);

  void initializeHardeningFunctions(unsigned qp);

  MooseSharedPointer<LinearInterpolation> _interp_yield_stress;
  const std::vector<FunctionName> _hardening_functions_names;
  std::vector<PiecewiseLinear *> _hardening_functions;
  std::vector<Real> _hf_temperatures;
  unsigned int _hf_index_lo;
  unsigned int _hf_index_hi;
  Real _hf_fraction;
};

template <>
InputParameters validParams<IsotropicTempDepHardening>();

#endif // ISOTROPICTEMPDEPHARDENING_H
