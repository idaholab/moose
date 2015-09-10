/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FINITESTRAINCPDISLOCATION_H
#define FINITESTRAINCPDISLOCATION_H

#include "FiniteStrainCPSlipRateRes.h"

/**
 * FiniteStrainCPDislocation is deriveed from FiniteStrainCrystalPlasticity.
 * Uses dislocation density based formulation.
 */

class FiniteStrainCPDislocation;

template<>
InputParameters validParams<FiniteStrainCPDislocation>();

class FiniteStrainCPDislocation : public FiniteStrainCPSlipRateRes
{

 public:
  FiniteStrainCPDislocation(const InputParameters & parameters);

 protected:
  /**
   * This function updates mobile dislocation densities.
   */
  virtual void update_mobile_disloc_density();
  /**
   * This function updates immobile dislocation densities.
   */
  virtual void update_immobile_disloc_density();
  /**
   * This function updates athermal slip system resitances.
   */
  virtual void update_athermal_resistance();
  /**
   * This function calculates slip increment due to climb.
   */
  virtual void get_glide_increments();
  /**
   * This function calculates slip incrments.
   */
  virtual void getSlipIncrements();

  virtual void initSlipSysProps();

  virtual void update_slip_system_resistance();

  virtual void readFileInitSlipSysRes();
  virtual void assignSlipSysRes();

  bool update_statevar( Real *, Real *, Real, Real );

  ///Number of internal variables of size nss
  unsigned int _num_internal_var_nss;
  ///Number of scalar internal variables
  unsigned int _num_internal_var_scalar;
  ///Residual tolerance when variable value is zero. Default 1e-12.
  Real _zero_tol;
  ///Penalty parameter value used to regularize activation energy based flow rule.
  Real _penalty_param;
  Real _b, _lg, _jump_freq, _enthal, _p, _q;
  Real _s_therm_v;
  Real _k, _temp;
  Real _q_p;
  Real _k_dyn;
  Real _r_c, _k_mul;
  Real _beta_rho;
  Real _self_harden, _latent_harden;
  bool _elastic_param_flag;
  Real _young_mod, _shear_mod;
  Real _rho_zero;

  ///Vector of internal variables
  MaterialProperty<std::vector<Real> > & _internal_var;
  MaterialProperty<std::vector<Real> > & _internal_var_old;

  MaterialProperty<RankTwoTensor> & _plastic_strain;
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  std::vector<Real> _slip_incr_glide;
  std::vector<Real> _dslipdtau_glide;
  std::vector<Real> _rho_m,_rho_m_old,_rho_m_prev;
  std::vector<Real> _rho_i,_rho_i_old,_rho_i_prev;

  std::vector<Real> _interaction_matrix;
  std::vector<Real> _s_therm, _s_atherm;
  std::vector<bool> _rho_m_evol_flag;

 private:

};


#endif //FINITESTRAINCPDISLOCATION_H
