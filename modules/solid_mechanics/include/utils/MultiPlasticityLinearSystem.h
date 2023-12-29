//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiPlasticityRawComponentAssembler.h"

/**
 * MultiPlasticityLinearSystem computes the linear system
 * and handles linear-dependence removal
 * for use in FiniteStrainMultiPlasticity
 *
 * Note that if run in debug mode you might have to use
 * the --no-trap-fpe flag because PETSc-LAPACK-BLAS
 * explicitly compute 0/0 and 1/0, and this causes
 * Libmesh to trap the floating-point exceptions
 *
 * These routines are quite complicated, so here is an
 * extended explanation.
 *
 *
 * SURFACES AND MODELS
 *
 * Each plasticity model can have multiple surfaces
 * (eg, Mohr-Coulomb has 6 surfaces), and one internal
 * parameter.  This is also described in
 * MultiPlasticityRawComponentAssembler.  The
 *
 *
 * VARIABLE NAMES
 *
 * _num_surfaces = total number of surfaces
 * _num_models = total number of plasticity models
 * pm = plasticity multiplier.  pm.size() = _num_surfaces
 * intnl = internal variable.  intnl.size() = _num_models
 *
 *
 * DEGREES OF FREEDOM
 *
 * The degrees of freedom are:
 * the 6 components of stress (it is assumed to be symmetric)
 * the plasticity multipliers, pm.
 * the internal parameters, intnl.
 *
 * Note that in any single Newton-Raphson (NR) iteration, the number
 * of pm and intnl may be different from any other NR iteration.
 * This is because of deactivating surfaces because they
 * are deemed unimportant by the calling program (eg, their
 * yield function is < 0), or because their flow direction is
 * linearly dependent on other surfaces.  Therefore:
 *
 * - there are "not active" surfaces, and these *never enter*
 * any right-hand-side (RHS), or jacobian, residual, etc calculations.
 * It is as if they never existed.
 *
 * - there are "active" surfaces, and *all these* enter RHS,
 * jacobian, etc, calculations.  This set of active surfaces
 * may further decompose into "linearly independent" and "linearly
 * dependent" surfaces.  The latter are surfaces whose flow direction
 * is linearly dependent on the former.  Only the dofs from
 * "linearly independent" constraints are changed during the NR step.
 *
 * Hence, more exactly, the degrees of freedom, whose changes will be
 * provided in the NR step are:
 * the 6 components of stress (it is assumed to be symmetric)
 * the plasticity multipliers, pm, belonging to linearly independent surfaces
 * the internal parameters, intnl, belonging to models with at least one linearly-independent
 * surface
 *
 *
 * THE CONSTRAINTS AND RHS
 *
 * The variables calculated by calculateConstraints and
 * calculateRHS are:
 * epp = pm*r - E_inv*(trial_stress - stress) = pm*r - delta_dp
 * f = yield function    [all the active constraints, including the deactivated_due_to_ld.  The
 * latter ones will not be put into the linear system]
 * ic = intnl - intnl_old + pm*h   [only for models that contain active surfaces]
 *
 * Here pm*r = sum_{active_alpha} pm[alpha]*r[alpha].  Note that this contains all the "active"
 * surfaces,
 *             even the ones that have been deactivated_due_to_ld.  in calculateConstraints, etc, r
 * is a
 *             std::vector containing only all the active flow directions (including
 *             deactivated_due_to_ld, but not the "not active").
 * f = all the "active" surfaces, even the ones that have been deactivated_due_to_ld.  However, the
 *             latter are not put into the RHS
 * pm*h = sum_{active_alpha} pm[alpha]*h[alpha].  Note that this only contains the "active"
 *             hardening potentials, even the ones that have been deactivated_due_to_ld.
 *             In calculateConstraints, calculateRHS and calculateJacobian,
 *             h is a std::vector containing only these "active" ones.
 *             Hence, the sum_{active_alpha} contains deactivated_due_to_ld contributions.
 *             HOWEVER, if all the surfaces belonging to a model are either
 *             "not active" or deactivated_due_to_ld, then this ic is not included in the RHS
 *
 * The RHS is
 * rhs = -(epp(0,0), epp(1,0), epp(1,1), epp(2,0), epp(2,1), epp(2,2), f[0], f[1], ...,
 * f[_num_active_f], ic[0], ic[1], ..., ic[num_active_ic])
 * Notice the appearance of only the i>=j "epp" components.
 *
 *
 * THE JACOBIAN
 *
 * This is d(-rhs)/d(dof).  Remember that the dofs are dependent
 * on what is deactivated_due_to_ld, as specified above.
 * In matrix form, the Jacobian is:
 * ( depp_dstress   depp_dpm  depp_dintnl )
 * (  df_dstress       0      df_dintnl   )
 * ( dic_dstress    dic_dpm   dic_dintnl  )
 * For the "epp" terms, only the i>=j components are kept in the RHS, so only these terms are kept
 * here too
 */
class MultiPlasticityLinearSystem : public MultiPlasticityRawComponentAssembler
{
public:
  static InputParameters validParams();

  MultiPlasticityLinearSystem(const MooseObject * moose_object);

protected:
  /// Tolerance on the minimum ratio of singular values before flow-directions are deemed linearly dependent
  Real _svd_tol;

  /// Minimum value of the _f_tol parameters for the Yield Function User Objects
  Real _min_f_tol;

  /**
   * The constraints.  These are set to zero (or <=0 in the case of the yield functions)
   * by the Newton-Raphson process, except in the case of linear-dependence which complicates
   * things.
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param pm Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param[out] f Active yield function(s)
   * @param[out] r Active flow directions
   * @param[out] epp Plastic-strain increment constraint
   * @param[out] ic Active internal-parameter constraint
   * @param active The active constraints.
   */
  virtual void calculateConstraints(const RankTwoTensor & stress,
                                    const std::vector<Real> & intnl_old,
                                    const std::vector<Real> & intnl,
                                    const std::vector<Real> & pm,
                                    const RankTwoTensor & delta_dp,
                                    std::vector<Real> & f,
                                    std::vector<RankTwoTensor> & r,
                                    RankTwoTensor & epp,
                                    std::vector<Real> & ic,
                                    const std::vector<bool> & active);

  /**
   * Calculate the RHS which is
   * rhs = -(epp(0,0), epp(1,0), epp(1,1), epp(2,0), epp(2,1), epp(2,2), f[0], f[1], ..., f[num_f],
   * ic[0], ic[1], ..., ic[num_ic])
   *
   * Note that the 'epp' components only contain the upper diagonal.  These contain flow directions
   * and plasticity-multipliers for all active surfaces, even the deactivated_due_to_ld surfaces.
   * Note that the 'f' components only contain the active and not deactivated_due_to_ld surfaces
   * Note that the 'ic' components only contain the internal constraints for models which contain
   * active and not deactivated_due_to_ld surfaces.  They contain hardening-potentials and
   * plasticity-multipliers for the active surfaces, even the deactivated_due_to_ld surfaces
   *
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param pm Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param[out] rhs the rhs
   * @param active The active constraints.
   * @param eliminate_ld Check for linear dependence of constraints and put the results into
   * deactivated_due_to_ld.  Usually this should be true, but for certain debug operations it should
   * be false
   * @param[out] deactivated_due_to_ld constraints deactivated due to linear-dependence of flow
   * directions
   */
  virtual void calculateRHS(const RankTwoTensor & stress,
                            const std::vector<Real> & intnl_old,
                            const std::vector<Real> & intnl,
                            const std::vector<Real> & pm,
                            const RankTwoTensor & delta_dp,
                            std::vector<Real> & rhs,
                            const std::vector<bool> & active,
                            bool eliminate_ld,
                            std::vector<bool> & deactivated_due_to_ld);

  /**
   * d(rhs)/d(dof)
   */
  virtual void calculateJacobian(const RankTwoTensor & stress,
                                 const std::vector<Real> & intnl,
                                 const std::vector<Real> & pm,
                                 const RankFourTensor & E_inv,
                                 const std::vector<bool> & active,
                                 const std::vector<bool> & deactivated_due_to_ld,
                                 std::vector<std::vector<Real>> & jac);

  /**
   * Performs one Newton-Raphson step.  The purpose here is to find the
   * changes, dstress, dpm and dintnl according to the Newton-Raphson procedure
   * @param stress Current value of stress
   * @param intnl_old The internal variables at the previous "time" step
   * @param intnl    Current value of the internal variables
   * @param pm  Current value of the plasticity multipliers (consistency parameters)
   * @param E_inv inverse of the elasticity tensor
   * @param delta_dp  Current value of the plastic-strain increment (ie plastic_strain -
   * plastic_strain_old)
   * @param[out] dstress The change in stress for a full Newton step
   * @param[out] dpm The change in all plasticity multipliers for a full Newton step
   * @param[out] dintnl The change in all internal variables for a full Newton step
   * @param active The active constraints
   * @param[out] deactivated_due_to_ld The constraints deactivated due to linear-dependence of the
   * flow directions
   */
  virtual void nrStep(const RankTwoTensor & stress,
                      const std::vector<Real> & intnl_old,
                      const std::vector<Real> & intnl,
                      const std::vector<Real> & pm,
                      const RankFourTensor & E_inv,
                      const RankTwoTensor & delta_dp,
                      RankTwoTensor & dstress,
                      std::vector<Real> & dpm,
                      std::vector<Real> & dintnl,
                      const std::vector<bool> & active,
                      std::vector<bool> & deactivated_due_to_ld);

private:
  /**
   * Performs a singular-value decomposition of r and returns the singular values
   *
   * Example: If r has size 5 then the singular values of the following matrix are returned:
   *     (  r[0](0,0) r[0](0,1) r[0](0,2) r[0](1,1) r[0](1,2) r[0](2,2)  )
   *     (  r[1](0,0) r[1](0,1) r[1](0,2) r[1](1,1) r[1](1,2) r[1](2,2)  )
   * a = (  r[2](0,0) r[2](0,1) r[2](0,2) r[2](1,1) r[2](1,2) r[2](2,2)  )
   *     (  r[3](0,0) r[3](0,1) r[3](0,2) r[3](1,1) r[3](1,2) r[3](2,2)  )
   *     (  r[4](0,0) r[4](0,1) r[4](0,2) r[4](1,1) r[4](1,2) r[4](2,2)  )
   *
   * @param r The flow directions
   * @param[out] s The singular values
   * @return The return value from the PETSc LAPACK gesvd reoutine
   */
  virtual int singularValuesOfR(const std::vector<RankTwoTensor> & r, std::vector<Real> & s);

  /**
   * Performs a number of singular-value decompositions
   * to check for linear-dependence of the active directions "r"
   * If linear dependence is found, then deactivated_due_to_ld will contain 'true' entries where
   * surfaces need to be deactivated_due_to_ld
   * @param stress the current stress
   * @param intnl the current values of internal parameters
   * @param f Active yield function values
   * @param r the flow directions that for those yield functions that are active upon entry to this
   * function
   * @param active true if active
   * @param[out] deactivated_due_to_ld Yield functions deactivated due to linearly-dependent flow
   * directions
   */
  virtual void eliminateLinearDependence(const RankTwoTensor & stress,
                                         const std::vector<Real> & intnl,
                                         const std::vector<Real> & f,
                                         const std::vector<RankTwoTensor> & r,
                                         const std::vector<bool> & active,
                                         std::vector<bool> & deactivated_due_to_ld);
};
