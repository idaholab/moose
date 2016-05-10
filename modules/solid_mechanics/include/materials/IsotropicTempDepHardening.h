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

/**
 */

class IsotropicTempDepHardening : public IsotropicPlasticity
{
public:
  IsotropicTempDepHardening(const InputParameters & parameters);

protected:
  virtual void computeYieldStress(unsigned qp);
  virtual void computeStressInitialize(unsigned qp, Real effectiveTrialStress, const SymmElasticityTensor & elasticityTensor);

  virtual void iterationFinalize(unsigned qp, Real scalar);

  virtual void updateHardeningFunction(unsigned qp);
  virtual Real computeHardening(unsigned qp, Real scalar);
  virtual Real computeHardeningVariable(unsigned qp, Real scalar);

  LinearInterpolation * _interp_yield_stress;
  MooseSharedPointer<LinearInterpolation> _interp_hardening_function;
  MooseSharedPointer<LinearInterpolation> _interp_hardening_function_old;
  std::vector<PiecewiseLinear *> _temp_dep_hardening_functions;
  std::vector<Real> _temp_dep_hardening_functions_temps;
  Real _stress_old;
};

template<>
InputParameters validParams<IsotropicTempDepHardening>();

#endif // ISOTROPICTEMPDEPHARDENING_H
