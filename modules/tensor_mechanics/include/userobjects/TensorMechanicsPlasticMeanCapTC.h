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
 * Rate-independent associative mean-cap tensile AND compressive failure
 * with hardening/softening of the tensile and compressive strength.
 * The key point here is that the internal parameter is equal to the
 * volumetric plastic strain.  This means that upon tensile failure, the
 * compressive strength can soften (using a TensorMechanicsHardening object)
 * which physically means that a subsequent compressive stress can
 * easily squash the material
 */
class TensorMechanicsPlasticMeanCapTC : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticMeanCapTC(const InputParameters & parameters);

  virtual void activeConstraints(const std::vector<Real> & f,
                                 const RankTwoTensor & stress,
                                 Real intnl,
                                 const RankFourTensor & Eijkl,
                                 std::vector<bool> & act,
                                 RankTwoTensor & returned_stress) const override;

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

  virtual std::string modelName() const override;

protected:
  /// max iters for custom return map loop
  const unsigned _max_iters;

  /// Whether to use the custom return-map algorithm
  const bool _use_custom_returnMap;

  /// Whether to use the custom consistent tangent operator algorithm
  const bool _use_custom_cto;

  /// the tensile strength
  const TensorMechanicsHardeningModel & _strength;

  /// the compressive strength
  const TensorMechanicsHardeningModel & _c_strength;

  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /**
   * The hardening potential.  Note that it is -1 for stress.trace() > _strength,
   * and +1 for stress.trace() < _c_strength.  This implements the idea that
   * tensile failure will cause a massive reduction in compressive strength
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl internal parameter
   * @return the hardening potential
   */
  Real hardPotential(const RankTwoTensor & stress, Real intnl) const override;

  virtual RankTwoTensor dhardPotential_dstress(const RankTwoTensor & stress,
                                               Real intnl) const override;

  virtual Real dhardPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /**
   * Derivative of the yield function with respect to stress.  This is also the flow potential.
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl internal parameter
   * @return the derivative
   */
  RankTwoTensor df_dsig(const RankTwoTensor & stress, Real intnl) const;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;

  /// compressive strength as a function of residual value, rate, and internal_param
  virtual Real compressive_strength(const Real internal_param) const;

  /// d(compressive strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dcompressive_strength(const Real internal_param) const;
};
