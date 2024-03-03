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

#include <array>

/**
 * TwoParameterPlasticityStressUpdate performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe (p, q) plasticity.  That is,
 * for plastic models where the yield function and flow
 * directions only depend on two parameters, p and q, that
 * are themselves functions of stress.
 */
class TwoParameterPlasticityStressUpdate : public MultiParameterPlasticityStressUpdate
{
public:
  static InputParameters validParams();

  TwoParameterPlasticityStressUpdate(const InputParameters & parameters,
                                     unsigned num_yf,
                                     unsigned num_intnl);

protected:
  /// Number of variables = 2 = (p, q)
  constexpr static int _num_pq = 2;

  /// Trial value of p
  Real _p_trial;

  /// Trial value of q
  Real _q_trial;

  /// elasticity tensor in p direction
  Real _Epp;

  /// elasticity tensor in q direction
  Real _Eqq;

  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dgaE_dpt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dgaE_dqt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dp_dpt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dq_dpt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dp_dqt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dq_dqt;

  /**
   * Computes the values of the yield functions, given p, q and intnl parameters.
   * Derived classes must override this, to provide the values of the yield functions
   * in yf.
   * @param p p stress
   * @param q q stress
   * @param intnl The internal parameters
   * @param[out] yf The yield function values
   */
  virtual void yieldFunctionValues(Real p,
                                   Real q,
                                   const std::vector<Real> & intnl,
                                   std::vector<Real> & yf) const = 0;
  void yieldFunctionValuesV(const std::vector<Real> & stress_params,
                            const std::vector<Real> & intnl,
                            std::vector<Real> & yf) const override;

  /**
   * Completely fills all_q with correct values.  These values are:
   * (1) the yield function values, yf[i]
   * (2) d(yf[i])/d(p, q)
   * (3) d(yf[i])/d(intnl[j])
   * (4) d(flowPotential[i])/d(p, q)
   * (5) d2(flowPotential[i])/d(p, q)/d(p, q)
   * (6) d2(flowPotential[i])/d(p, q)/d(intnl[j])
   * @param p p stress
   * @param q q stress
   * @param intnl The internal parameters
   * @param[out] all_q All the desired quantities
   */
  virtual void computeAllQ(Real p,
                           Real q,
                           const std::vector<Real> & intnl,
                           std::vector<yieldAndFlow> & all_q) const = 0;
  void computeAllQV(const std::vector<Real> & stress_params,
                    const std::vector<Real> & intnl,
                    std::vector<yieldAndFlow> & all_q) const override;

  /**
   * Derived classes may employ this function to record stuff or do
   * other computations prior to the return-mapping algorithm.  We
   * know that (p_trial, q_trial, intnl_old) is inadmissible.
   * @param p_trial Trial value of p
   * @param q_trial Trial value of q
   * @param stress_trial Trial stress tensor
   * @param intnl_old Old value of the internal parameters.
   * @param yf The yield functions at (p_trial, q_trial, intnl_old)
   * @param Eijkl The elasticity tensor
   */
  virtual void preReturnMap(Real p_trial,
                            Real q_trial,
                            const RankTwoTensor & stress_trial,
                            const std::vector<Real> & intnl_old,
                            const std::vector<Real> & yf,
                            const RankFourTensor & Eijkl);
  void preReturnMapV(const std::vector<Real> & trial_stress_params,
                     const RankTwoTensor & stress_trial,
                     const std::vector<Real> & intnl_old,
                     const std::vector<Real> & yf,
                     const RankFourTensor & Eijkl) override;

  /**
   * Sets (p, q, gaE, intnl) at "good guesses" of the solution to the Return-Map algorithm.
   * The purpose of these "good guesses" is to speed up the Newton-Raphson process by providing
   * it with a good initial guess.
   * Derived classes may choose to override this if their plastic models are easy
   * enough to solve approximately.
   * The default values, provided by this class, are simply p=p_trial, etc: that is,
   * the "good guess" is just the trial point for this (sub)strain increment.
   * @param p_trial The trial value of p for this (sub)strain increment
   * @param q_trial The trial value of q for this (sub)strain increment
   * @param intnl_old The internal parameters before applying the (sub)strain increment
   * @param p[out] The "good guess" value of p.  Default = p_trial
   * @param q[out] The "good guess" value of q.  Default = q_trial
   * @param gaE[out] The "good guess" value of gaE.  Default = 0
   * @param intnl[out] The "good guess" value of the internal parameters
   */
  virtual void initializeVars(Real p_trial,
                              Real q_trial,
                              const std::vector<Real> & intnl_old,
                              Real & p,
                              Real & q,
                              Real & gaE,
                              std::vector<Real> & intnl) const;
  void initializeVarsV(const std::vector<Real> & trial_stress_params,
                       const std::vector<Real> & intnl_old,
                       std::vector<Real> & stress_params,
                       Real & gaE,
                       std::vector<Real> & intnl) const override;

  /**
   * Sets the internal parameters based on the trial values of
   * p and q, their current values, and the old values of the
   * internal parameters.
   * Derived classes must override this.
   * @param p_trial Trial value of p
   * @param q_trial Trial value of q
   * @param p Current value of p
   * @param q Current value of q
   * @param intnl_old Old value of internal parameters
   * @param intnl[out] The value of internal parameters to be set
   */
  virtual void setIntnlValues(Real p_trial,
                              Real q_trial,
                              Real p,
                              Real q,
                              const std::vector<Real> & intnl_old,
                              std::vector<Real> & intnl) const = 0;
  void setIntnlValuesV(const std::vector<Real> & trial_stress_params,
                       const std::vector<Real> & current_stress_params,
                       const std::vector<Real> & intnl_old,
                       std::vector<Real> & intnl) const override;

  /**
   * Sets the derivatives of internal parameters, based on the trial values of
   * p and q, their current values, and the old values of the
   * internal parameters.
   * Derived classes must override this.
   * @param p_trial Trial value of p
   * @param q_trial Trial value of q
   * @param p Current value of p
   * @param q Current value of q
   * @param intnl The current value of the internal parameters
   * @param dintnl The derivatives dintnl[i][j] = d(intnl[i])/d(variable j), where variable0=p and
   * variable1=q
   */
  virtual void setIntnlDerivatives(Real p_trial,
                                   Real q_trial,
                                   Real p,
                                   Real q,
                                   const std::vector<Real> & intnl,
                                   std::vector<std::vector<Real>> & dintnl) const = 0;
  virtual void setIntnlDerivativesV(const std::vector<Real> & trial_stress_params,
                                    const std::vector<Real> & current_stress_params,
                                    const std::vector<Real> & intnl,
                                    std::vector<std::vector<Real>> & dintnl) const override;

  /**
   * Computes p and q, given stress.  Derived classes must
   * override this
   * @param stress Stress tensor
   * @param p p stress
   * @param q q q stress
   */
  virtual void computePQ(const RankTwoTensor & stress, Real & p, Real & q) const = 0;
  virtual void computeStressParams(const RankTwoTensor & stress,
                                   std::vector<Real> & stress_params) const override;

  /**
   * Set Epp and Eqq based on the elasticity tensor
   * Derived classes must override this
   * @param Eijkl elasticity tensor
   * @param[out] Epp Epp value
   * @param[out] Eqq Eqq value
   */
  virtual void setEppEqq(const RankFourTensor & Eijkl, Real & Epp, Real & Eqq) const = 0;
  virtual void setEffectiveElasticity(const RankFourTensor & Eijkl) override;

  /**
   * Sets stress from the admissible parameters.
   * This is called after the return-map process has completed
   * successfully in (p, q) space, just after finalizeReturnProcess
   * has been called.
   * Derived classes may override this function
   * @param stress_trial The trial value of stress
   * @param p_ok Returned value of p
   * @param q_ok Returned value of q
   * @param gaE Value of gaE induced by the return (gaE = gamma * Epp)
   * @param smoothed_q Holds the current value of yield function and derivatives evaluated at (p_ok,
   * q_ok, _intnl)
   * @param Eijkl The elasticity tensor
   * @param stress[out] The returned value of the stress tensor
   */
  virtual void setStressAfterReturn(const RankTwoTensor & stress_trial,
                                    Real p_ok,
                                    Real q_ok,
                                    Real gaE,
                                    const std::vector<Real> & intnl,
                                    const yieldAndFlow & smoothed_q,
                                    const RankFourTensor & Eijkl,
                                    RankTwoTensor & stress) const = 0;
  void setStressAfterReturnV(const RankTwoTensor & stress_trial,
                             const std::vector<Real> & stress_params,
                             Real gaE,
                             const std::vector<Real> & intnl,
                             const yieldAndFlow & smoothed_q,
                             const RankFourTensor & Eijkl,
                             RankTwoTensor & stress) const override;

  void
  setInelasticStrainIncrementAfterReturn(const RankTwoTensor & stress_trial,
                                         Real gaE,
                                         const yieldAndFlow & smoothed_q,
                                         const RankFourTensor & elasticity_tensor,
                                         const RankTwoTensor & returned_stress,
                                         RankTwoTensor & inelastic_strain_increment) const override;

  /**
   * Calculates the consistent tangent operator.
   * Derived classes may choose to override this for computational
   * efficiency.  The implementation in this class is quite expensive,
   * even though it looks compact and clean, because of all the
   * manipulations of RankFourTensors involved.
   * @param stress_trial the trial value of the stress tensor for this strain increment
   * @param p_trial the trial value of p for this strain increment
   * @param q_trial the trial value of q for this strain increment
   * @param stress the returned value of the stress tensor for this strain increment
   * @param p the returned value of p for this strain increment
   * @param q the returned value of q for this strain increment
   * @param gaE the total value of that came from this strain increment
   * @param smoothed_q contains the yield function and derivatives evaluated at (p, q)
   * @param Eijkl The elasticity tensor
   * @param compute_full_tangent_operator true if the full consistent tangent operator is needed,
   * otherwise false
   * @param[out] cto The consistent tangent operator
   */
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
                                         RankFourTensor & cto) const;

  void consistentTangentOperatorV(const RankTwoTensor & stress_trial,
                                  const std::vector<Real> & trial_stress_params,
                                  const RankTwoTensor & stress,
                                  const std::vector<Real> & stress_params,
                                  Real gaE,
                                  const yieldAndFlow & smoothed_q,
                                  const RankFourTensor & Eijkl,
                                  bool compute_full_tangent_operator,
                                  const std::vector<std::vector<Real>> & dvar_dtrial,
                                  RankFourTensor & cto) override;

  virtual std::vector<RankTwoTensor>
  dstress_param_dstress(const RankTwoTensor & stress) const override;
  virtual std::vector<RankFourTensor>
  d2stress_param_dstress(const RankTwoTensor & stress) const override;
  /**
   * d(p)/d(stress)
   * Derived classes must override this
   * @param stress stress tensor
   * @return d(p)/d(stress)
   */
  virtual RankTwoTensor dpdstress(const RankTwoTensor & stress) const = 0;

  /**
   * d2(p)/d(stress)/d(stress)
   * Derived classes must override this
   * @param stress stress tensor
   * @return d2(p)/d(stress)/d(stress)
   */
  virtual RankFourTensor d2pdstress2(const RankTwoTensor & stress) const = 0;

  /**
   * d(q)/d(stress)
   * Derived classes must override this
   * @param stress stress tensor
   * @return d(q)/d(stress)
   */
  virtual RankTwoTensor dqdstress(const RankTwoTensor & stress) const = 0;

  /**
   * d2(q)/d(stress)/d(stress)
   * Derived classes must override this
   * @param stress stress tensor
   * @return d2(q)/d(stress)/d(stress)
   */
  virtual RankFourTensor d2qdstress2(const RankTwoTensor & stress) const = 0;
};
