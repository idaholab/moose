/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RECOMPUTERADIALRETURNTEMPDEPHARDENING_H
#define RECOMPUTERADIALRETURNTEMPDEPHARDENING_H

#include "RecomputeRadialReturnIsotropicPlasticity.h"

class PiecewiseLinear;
class LinearInterpolation;

/**
 * This class inherits from RecomputeRadialReturnIsotropicPlasticity. It
 * calculates stress as a function of temperature and plastic strain by
 * interpolating hardening functions at different temperatures input by the user.
 */

class RecomputeRadialReturnTempDepHardening : public RecomputeRadialReturnIsotropicPlasticity
{
public:
  RecomputeRadialReturnTempDepHardening(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(Real effectiveTrialStress);

  virtual void computeYieldStress();
  virtual Real computeHardeningValue(Real scalar);
  virtual Real computeHardeningDerivative(Real scalar);

  void initializeHardeningFunctions();

  MooseSharedPointer<LinearInterpolation> _interp_yield_stress;
  const std::vector<FunctionName> _hardening_functions_names;
  std::vector<PiecewiseLinear *> _hardening_functions;
  std::vector<Real> _hf_temperatures;
  unsigned int _hf_index;
  Real _hf_fraction;
};

template<>
InputParameters validParams<RecomputeRadialReturnTempDepHardening>();

#endif //RECOMPUTERADIALRETURNTEMPDEPHARDENING_H
