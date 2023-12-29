//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoParameterPlasticityStressUpdate.h"
#include "TensorMechanicsHardeningModel.h"
#include "TensorMechanicsPlasticDruckerPrager.h"

/**
 * CappedDruckerPragerStressUpdate performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped Drucker-Prager plasticity.
 *
 * This plastic model is ideally suited to a material (eg rock)
 * that has different compressive, tensile and shear
 * strengths.  Any of these strengths may be set large to
 * accommodate special situations.  For instance, setting
 * the compressive strength large is appropriate if there
 * is no chance of compressive failure in a model.
 *
 * This plastic model has two internal parameters:
 *  - a "shear" internal parameter which parameterises the
 *    amount of shear plastic strain.  The cohesion, friction
 *    angle, and dilation angle are assumed to be functions
 *    of this internal parameter.
 *  - a "tensile" internal parameter which parameterises the
 *    amount of tensile plastic strain.  The tensile strength
 *    and compressive strength are assumed to be functions of
 *    this internal parameter.  This means, for instance, that
 *    failure in tension can cause the compressive strenght
 *    to soften, as would be expected in rock-mechanics
 *    scenarios.
 *
 * This plasticity model assumes the elasticity tensor obeys:
 * E_ijkl s_kl = 2 E_0101 * s_ij, for any traceless tensor, s_ij.
 * (This expression defines the scalar quantity, Eqq,
 *  which is just the shear modulus in the isotropic situation.)
 *
 * As of Feb2017, small tests suggest that this Material model
 * is marginally more efficient than ComputeMultiPlasticityStress
 * with a DruckerPragerHyperbolic user object, if the compressive
 * and tensile strengths are set large.  Adding compressive or
 * tensile failure adds about 40% more computation time (on
 * average) which again appears to be better than
 * ComputeMultiPlasticityStress with DruckerPrager and MeanCap(s)
 * user objects.
 */
class CappedDruckerPragerStressUpdate : public TwoParameterPlasticityStressUpdate
{
public:
  static InputParameters validParams();

  CappedDruckerPragerStressUpdate(const InputParameters & parameters);

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return true; }

  bool isIsotropic() override { return true; };

protected:
  /// Hardening model for cohesion, friction and dilation angles
  const TensorMechanicsPlasticDruckerPrager & _dp;

  /// Hardening model for tensile strength
  const TensorMechanicsHardeningModel & _tstrength;

  /// Hardening model for compressive strength
  const TensorMechanicsHardeningModel & _cstrength;

  /// The cone vertex is smoothed by this amount
  const Real _small_smoother2;

  /// Initialize the NR proceedure from a guess coming from perfect plasticity
  const bool _perfect_guess;

  /**
   * This allows some simplification in the return-map process.
   * If the tensile yield function yf[1] >= 0 at the trial stress then
   * clearly the quadpoint is not going to fail in compression, so
   * _stress_return_type is set to no_compression, and every time
   * the compressive yield function is evaluated it is set -(very large).
   * If the compressive yield function yf[2] >= 0 at the trial stress
   * then clearly the quadpoint is not going to fail in tension, so
   * _stress_return_type is set to no_tension, and every time the
   * tensile yield function is evaluated it is set -(very large).
   * Otherwise (and at the very end after return-map) _stress_return_type
   * is set to nothing_special.
   */
  enum class StressReturnType
  {
    nothing_special,
    no_compression,
    no_tension
  } _stress_return_type;

  /**
   * If true, and if the trial stress exceeds the tensile strength,
   * then the user gaurantees that the returned stress will be
   * independent of the compressive strength.
   */
  const bool _small_dilation;

  /// trial value of q
  Real _in_q_trial;

  virtual void yieldFunctionValues(Real p,
                                   Real q,
                                   const std::vector<Real> & intnl,
                                   std::vector<Real> & yf) const override;

  virtual void computeAllQ(Real p,
                           Real q,
                           const std::vector<Real> & intnl,
                           std::vector<yieldAndFlow> & all_q) const override;

  virtual void preReturnMap(Real p_trial,
                            Real q_trial,
                            const RankTwoTensor & stress_trial,
                            const std::vector<Real> & intnl_old,
                            const std::vector<Real> & yf,
                            const RankFourTensor & Eijkl) override;

  virtual void initializeVars(Real p_trial,
                              Real q_trial,
                              const std::vector<Real> & intnl_old,
                              Real & p,
                              Real & q,
                              Real & gaE,
                              std::vector<Real> & intnl) const override;

  virtual void setIntnlValues(Real p_trial,
                              Real q_trial,
                              Real p,
                              Real q,
                              const std::vector<Real> & intnl_old,
                              std::vector<Real> & intnl) const override;

  virtual void setIntnlDerivatives(Real p_trial,
                                   Real q_trial,
                                   Real p,
                                   Real q,
                                   const std::vector<Real> & intnl,
                                   std::vector<std::vector<Real>> & dintnl) const override;

  virtual void computePQ(const RankTwoTensor & stress, Real & p, Real & q) const override;

  virtual void initializeReturnProcess() override;

  virtual void finalizeReturnProcess(const RankTwoTensor & rotation_increment) override;

  virtual void setEppEqq(const RankFourTensor & Eijkl, Real & Epp, Real & Eqq) const override;

  virtual void setStressAfterReturn(const RankTwoTensor & stress_trial,
                                    Real p_ok,
                                    Real q_ok,
                                    Real gaE,
                                    const std::vector<Real> & intnl,
                                    const yieldAndFlow & smoothed_q,
                                    const RankFourTensor & Eijkl,
                                    RankTwoTensor & stress) const override;

  virtual void consistentTangentOperator(const RankTwoTensor & stress_trial,
                                         Real p_trial,
                                         Real q_trial,
                                         const RankTwoTensor & stress,
                                         Real p,
                                         Real q,
                                         Real gaE,
                                         const yieldAndFlow & smoothed_q,
                                         const RankFourTensor & Eijkl,
                                         bool compute_full_tangent_operator,
                                         RankFourTensor & cto) const override;

  virtual RankTwoTensor dpdstress(const RankTwoTensor & stress) const override;

  virtual RankFourTensor d2pdstress2(const RankTwoTensor & stress) const override;

  virtual RankTwoTensor dqdstress(const RankTwoTensor & stress) const override;

  virtual RankFourTensor d2qdstress2(const RankTwoTensor & stress) const override;
};
