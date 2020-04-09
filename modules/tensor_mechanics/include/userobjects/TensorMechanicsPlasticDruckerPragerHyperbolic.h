//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticDruckerPrager.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * Rate-independent non-associative Drucker Prager
 * with hardening/softening.  The cone's tip is smoothed in a hyperbolic fashion
 * Most functions (eg flowPotential, etc) are simply inherited from
 * TensorMechanicsPlasticDruckerPrager.  Note df_dsig is over-ridden
 */
class TensorMechanicsPlasticDruckerPragerHyperbolic : public TensorMechanicsPlasticDruckerPrager
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticDruckerPragerHyperbolic(const InputParameters & parameters);

  virtual std::string modelName() const override;

  virtual bool useCustomReturnMap() const override;

  virtual bool useCustomCTO() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  /// Function that's used in dyieldFunction_dstress and flowPotential
  virtual RankTwoTensor df_dsig(const RankTwoTensor & stress, Real bbb) const override;

  virtual bool returnMap(const RankTwoTensor & trial_stress,
                         Real intnl_old,
                         const RankFourTensor & E_ijkl,
                         Real ep_plastic_tolerance,
                         RankTwoTensor & returned_stress,
                         Real & returned_intnl,
                         std::vector<Real> & dpm,
                         RankTwoTensor & delta_dp,
                         std::vector<Real> & yf,
                         bool & trial_stress_inadmissible) const override;

  virtual RankFourTensor
  consistentTangentOperator(const RankTwoTensor & trial_stress,
                            Real intnl_old,
                            const RankTwoTensor & stress,
                            Real intnl,
                            const RankFourTensor & E_ijkl,
                            const std::vector<Real> & cumulative_pm) const override;

private:
  /// smoothing parameter for the cone's tip
  const Real _smoother2;

  /// whether to use the custom returnMap function
  const bool _use_custom_returnMap;

  /// Whether to use the custom consistent tangent operator calculation
  const bool _use_custom_cto;

  /// max iters for custom return map loop
  const unsigned _max_iters;
};
