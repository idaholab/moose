#ifndef MULTIPLASTICITYLINEARSYSTEM_H
#define MULTIPLASTICITYLINEARSYSTEM_H

#include "MultiPlasticityRawComponentAssembler.h"

class MultiPlasticityLinearSystem;

template<>
InputParameters validParams<MultiPlasticityLinearSystem>();

/**
 * MultiPlasticityLinearSystem computes the linear system
 * and handles linear-dependence removal
 * for use in FiniteStrainMultiPlasticity
 *
 * Note that if run in debug mode you might have to use
 * the --no-trap-fpe flag because PETSc-LAPACK-BLAS
 * explicitly compute 0/0 and 1/0, and this causes
 * Libmesh to trap the floating-point exceptions
 */
class MultiPlasticityLinearSystem:
  public MultiPlasticityRawComponentAssembler
{
public:
  MultiPlasticityLinearSystem(const std::string & name, InputParameters parameters);

protected:

  /// Tolerance on the minimum ratio of singular values before flow-directions are deemed linearly dependent
  Real _svd_tol;

  /// Minimum value of the _f_tol parameters for the Yield Function User Objects
  Real _min_f_tol;


  /**
   * The constraints.  These are set to zero (or <=0 in the case of the yield functions)
   * by the Newton-Raphson process, except in the case of linear-dependence which complicates things.
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param pm Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param f (output) Active yield function(s)
   * @param r (output) Active flow directions
   * @param epp (output) Plastic-strain increment constraint
   * @param ic (output) Active internal-parameter constraint
   * @param active The active constraints.
   */
  virtual void calculateConstraints(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & f, std::vector<RankTwoTensor> & r, RankTwoTensor & epp, std::vector<Real> & ic, const std::vector<bool> & active);


  /**
   * Calculate the RHS which is
   * rhs = -(epp(0,0), epp(1,0), epp(1,1), epp(2,0), epp(2,1), epp(2,2), f[0], f[1], ..., f[num_f], ic[0], ic[1], ..., ic[num_ic])
   *
   * Note that the 'epp' components only contain the upper diagonal.  These contain flow directions and plasticity-multipliers for all active surfaces, even the deactivated_due_to_ld surfaces.
   * Note that the 'f' components only contain the active and not deactivated_due_to_ld surfaces
   * Note that the 'ic' components only contain the internal constraints for models which contain active and not deactivated_due_to_ld surfaces.  They contain hardening-potentials and plasticity-multipliers for the active surfaces, even the deactivated_due_to_ld surfaces
   *
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param pm Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param rhs (output) the rhs
   * @param active The active constraints.
   * @param eliminate_ld Check for linear dependence of constraints and put the results into deactivated_due_to_ld.  Usually this should be true, but for certain debug operations it should be false
   * @param deactivated_due_to_ld (output) constraints deactivated due to linear-dependence of flow directions
   */
  virtual void calculateRHS(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & rhs, const std::vector<bool> & active, bool eliminate_ld, std::vector<bool> & deactivated_due_to_ld);

  /**
   * d(rhs)/d(dof)
   */
  virtual void calculateJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, const std::vector<bool> & active, const std::vector<bool> & deactivated_due_to_ld, std::vector<std::vector<Real> > & jac);

  /**
   * Performs one Newton-Raphson step.  The purpose here is to find the
   * changes, dstress, dpm and dintnl according to the Newton-Raphson procedure
   * @param stress Current value of stress
   * @param intnl_old The internal variables at the previous "time" step
   * @param intnl    Current value of the internal variables
   * @param pm  Current value of the plasticity multipliers (consistency parameters)
   * @param E_inv inverse of the elasticity tensor
   * @param delta_dp  Current value of the plastic-strain increment (ie plastic_strain - plastic_strain_old)
   * @param dstress (output) The change in stress for a full Newton step
   * @param dpm (output) The change in all plasticity multipliers for a full Newton step
   * @param dintnl (output) The change in all internal variables for a full Newton step
   * @param active The active constraints
   * @param deactivated_due_to_ld (output) The constraints deactivated due to linear-dependence of the flow directions
   */
  virtual void nrStep(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, const RankTwoTensor & delta_dp, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl, const std::vector<bool> & active, std::vector<bool> & deactivated_due_to_ld);


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
   * @param s (output) The singular values
   * @return The return value from the PETSc LAPACK gesvd reoutine
   */
  virtual int singularValuesOfR(const std::vector<RankTwoTensor> & r, std::vector<Real> & s);

  /**
   * Performs a number of singular-value decompositions
   * to check for linear-dependence of the active directions "r"
   * If linear dependence is found, then deactivated_due_to_ld will contain 'true' entries where surfaces need to be deactivated_due_to_ld
   * @param stress the current stress
   * @param intnl the current values of internal parameters
   * @param f Active yield function values
   * @param r the flow directions that for those yield functions that are active upon entry to this function
   * @param active true if active
   * @param (output) deactivated_due_to_ld Yield functions deactivated due to linearly-dependent flow directions
   */
  virtual void eliminateLinearDependence(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<Real> & f, const std::vector<RankTwoTensor> & r, const std::vector<bool> & active, std::vector<bool> & deactivated_due_to_ld);


};

#endif //MULTIPLASTICITYLINEARSYSTEM_H
