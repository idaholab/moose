/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTIPLASTICITYDEBUGGER_H
#define MULTIPLASTICITYDEBUGGER_H

#include "MultiPlasticityLinearSystem.h"

class MultiPlasticityDebugger;

template <>
InputParameters validParams<MultiPlasticityDebugger>();

/**
 * MultiPlasticityDebugger computes various
 * finite-difference things to help developers
 * remove bugs in their derivatives, etc.
 */
class MultiPlasticityDebugger : public MultiPlasticityLinearSystem
{
public:
  MultiPlasticityDebugger(const MooseObject * moose_object);

  /**
   * Outputs the debug parameters: _fspb_debug_stress, _fspd_debug_pm, etc
   * and checks that they are sized correctly
   */
  void outputAndCheckDebugParameters();

  /**
   * Checks the derivatives, eg dyieldFunction_dstress by using
   * finite difference approximations.
   */
  void checkDerivatives();

  /**
   * Checks the full Jacobian, which is just certain
   * linear combinations of the dyieldFunction_dstress, etc,
   * by using finite difference approximations
   */
  void checkJacobian(const RankFourTensor & E_inv, const std::vector<Real> & intnl_old);

  /**
   * Checks that Ax does equal b in the NR procedure
   */
  void checkSolution(const RankFourTensor & E_inv);

protected:
  /**
   * none - don't do any debugging
   * crash - currently inactive
   * jacobian - check the jacobian entries
   * jacobian_and_linear_system - check entire jacobian and check that Ax=b
   */
  MooseEnum _fspb_debug;

  /// Debug the Jacobian entries at this stress
  RankTwoTensor _fspb_debug_stress;

  /// Debug the Jacobian entires at these plastic multipliers
  std::vector<Real> _fspb_debug_pm;

  /// Debug the Jacobian entires at these internal parameters
  std::vector<Real> _fspb_debug_intnl;

  /// Debug finite-differencing parameter for the stress
  Real _fspb_debug_stress_change;

  /// Debug finite-differencing parameters for the plastic multipliers
  std::vector<Real> _fspb_debug_pm_change;

  /// Debug finite-differencing parameters for the internal parameters
  std::vector<Real> _fspb_debug_intnl_change;

private:
  /**
   * The finite-difference derivative of yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param[out] df_dstress the derivative (or derivatives in the case of multisurface plasticity).
   * df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  void fddyieldFunction_dstress(const RankTwoTensor & stress,
                                const std::vector<Real> & intnl,
                                std::vector<RankTwoTensor> & df_dstress);

  /**
   * The finite-difference derivative of yield function(s) with respect to internal parameter(s)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param[out] df_dintnl the derivative (or derivatives in the case of multisurface plasticity).
   * df_dintnl[alpha] = dyieldFunction[alpha]/dintnl[alpha]
   */
  void fddyieldFunction_dintnl(const RankTwoTensor & stress,
                               const std::vector<Real> & intnl,
                               std::vector<Real> & df_dintnl);

  /**
   * The finite-difference derivative of the flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param[out] dr_dstress the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i,
   * j)/dstress(k, l)
   */
  virtual void fddflowPotential_dstress(const RankTwoTensor & stress,
                                        const std::vector<Real> & intnl,
                                        std::vector<RankFourTensor> & dr_dstress);

  /**
   * The finite-difference derivative of the flow potentials with respect to internal parameters
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param[out] dr_dintnl the derivatives.  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl[alpha]
   */
  virtual void fddflowPotential_dintnl(const RankTwoTensor & stress,
                                       const std::vector<Real> & intnl,
                                       std::vector<RankTwoTensor> & dr_dintnl);

  /**
   * The Jacobian calculated using finite differences.  The output
   * should be equal to calculateJacobian(...) if everything is
   * coded correctly.
   * @param stress the stress at which to calculate the Jacobian
   * @param intnl_old the old values of internal variables (jacobian is inependent of these, but
   * they are needed to do the finite-differencing cleanly)
   * @param intnl the vector of internal parameters at which to calculate the Jacobian
   * @param pm the plasticity multipliers at which to calculate the Jacobian
   * @param delta_dp plastic_strain - plastic_strain_old (Jacobian is independent of this, but it is
   * needed to do the finite-differencing cleanly)
   * @param E_inv inverse of the elasticity tensor
   * @param eliminate_ld only calculate the Jacobian for the linearly independent constraints
   * @param[out] jac the finite-difference Jacobian
   */
  virtual void fdJacobian(const RankTwoTensor & stress,
                          const std::vector<Real> & intnl_old,
                          const std::vector<Real> & intnl,
                          const std::vector<Real> & pm,
                          const RankTwoTensor & delta_dp,
                          const RankFourTensor & E_inv,
                          bool eliminate_ld,
                          std::vector<std::vector<Real>> & jac);

  bool dof_included(unsigned int dof, const std::vector<bool> & deactivated_due_to_ld);
};

#endif // MULTIPLASTICITYDEBUGGER_H
