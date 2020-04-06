//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiParameterPlasticityStressUpdate.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * TensileStressUpdate implements rate-independent associative tensile failure
 * ("Rankine" plasticity) with hardening/softening.
 */
class TensileStressUpdate : public MultiParameterPlasticityStressUpdate
{
public:
  static InputParameters validParams();

  TensileStressUpdate(const InputParameters & parameters);

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return true; }

protected:
  /// Hardening model for tensile strength
  const TensorMechanicsHardeningModel & _strength;

  /// Whether to provide an estimate of the returned stress, based on perfect plasticity
  const bool _perfect_guess;

  /// Eigenvectors of the trial stress as a RankTwoTensor, in order to rotate the returned stress back to stress space
  RankTwoTensor _eigvecs;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;

  void computeStressParams(const RankTwoTensor & stress,
                           std::vector<Real> & stress_params) const override;

  std::vector<RankTwoTensor> dstress_param_dstress(const RankTwoTensor & stress) const override;

  std::vector<RankFourTensor> d2stress_param_dstress(const RankTwoTensor & stress) const override;

  virtual void setStressAfterReturnV(const RankTwoTensor & stress_trial,
                                     const std::vector<Real> & stress_params,
                                     Real gaE,
                                     const std::vector<Real> & intnl,
                                     const yieldAndFlow & smoothed_q,
                                     const RankFourTensor & Eijkl,
                                     RankTwoTensor & stress) const override;

  virtual void preReturnMapV(const std::vector<Real> & trial_stress_params,
                             const RankTwoTensor & stress_trial,
                             const std::vector<Real> & intnl_old,
                             const std::vector<Real> & yf,
                             const RankFourTensor & Eijkl) override;

  void setEffectiveElasticity(const RankFourTensor & Eijkl) override;

  void yieldFunctionValuesV(const std::vector<Real> & stress_params,
                            const std::vector<Real> & intnl,
                            std::vector<Real> & yf) const override;

  void computeAllQV(const std::vector<Real> & stress_params,
                    const std::vector<Real> & intnl,
                    std::vector<yieldAndFlow> & all_q) const override;

  void initializeVarsV(const std::vector<Real> & trial_stress_params,
                       const std::vector<Real> & intnl_old,
                       std::vector<Real> & stress_params,
                       Real & gaE,
                       std::vector<Real> & intnl) const override;

  void setIntnlValuesV(const std::vector<Real> & trial_stress_params,
                       const std::vector<Real> & current_stress_params,
                       const std::vector<Real> & intnl_old,
                       std::vector<Real> & intnl) const override;

  void setIntnlDerivativesV(const std::vector<Real> & trial_stress_params,
                            const std::vector<Real> & current_stress_params,
                            const std::vector<Real> & intnl,
                            std::vector<std::vector<Real>> & dintnl) const override;

  virtual void consistentTangentOperatorV(const RankTwoTensor & stress_trial,
                                          const std::vector<Real> & trial_stress_params,
                                          const RankTwoTensor & stress,
                                          const std::vector<Real> & stress_params,
                                          Real gaE,
                                          const yieldAndFlow & smoothed_q,
                                          const RankFourTensor & Eijkl,
                                          bool compute_full_tangent_operator,
                                          const std::vector<std::vector<Real>> & dvar_dtrial,
                                          RankFourTensor & cto) override;
};
