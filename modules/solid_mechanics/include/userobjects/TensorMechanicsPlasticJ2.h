//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * J2 plasticity, associative, with hardning.
 * Yield_function = sqrt(3*J2) - yield_strength
 */
class TensorMechanicsPlasticJ2 : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticJ2(const InputParameters & parameters);

  virtual std::string modelName() const override;

  virtual bool useCustomReturnMap() const override;

  virtual bool useCustomCTO() const override;

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

protected:
  virtual Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  virtual RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress,
                                               Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  virtual RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  virtual RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress,
                                                Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /**
   * YieldStrength.  The yield function is sqrt(3*J2) - yieldStrength.
   * In this class yieldStrength = 1, but this
   * may be over-ridden by derived classes with nontrivial hardning
   */
  virtual Real yieldStrength(Real intnl) const;

  /// d(yieldStrength)/d(intnl)
  virtual Real dyieldStrength(Real intnl) const;

private:
  /// yield strength, from user input
  const TensorMechanicsHardeningModel & _strength;

  /// max iters for custom return map loop
  const unsigned _max_iters;

  /// Whether to use the custom return-map algorithm
  const bool _use_custom_returnMap;

  /// Whether to use the custom consistent tangent operator calculation
  const bool _use_custom_cto;
};
