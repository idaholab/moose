#ifndef FINITESTRAINMULTIPLASTICITY
#define FINITESTRAINMULTIPLASTICITY

#include "FiniteStrainMaterial.h"
#include "TensorMechanicsPlasticModel.h"

class FiniteStrainMultiPlasticity;

template<>
InputParameters validParams<FiniteStrainMultiPlasticity>();

/**
 * FiniteStrainMultiPlasticity is performs the return-map
 * algorithm and associated stress updates for plastic
 * models defined by a General User Objects
 */
class FiniteStrainMultiPlasticity : public FiniteStrainMaterial
{
public:
  FiniteStrainMultiPlasticity(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  /// Maximum number of Newton-Raphson iterations allowed
  unsigned int _max_iter;

  /// Maximum number of subdivisions allowed
  unsigned int _max_subdivisions;

  /// Number of plastic models for this material
  unsigned int _num_f;

  /// Tolerance on the plastic strain increment ("direction") constraint
  Real _epp_tol;

  /// Tolerance on the minimum ratio of singular values before flow-directions are deemed linearly dependent
  Real _svd_tol;

  /// Minimum value of the _f_tol parameters for the Yield Function User Objects
  Real _min_f_tol;

  /// When in the Newton-Raphson to deactivate constraints
  MooseEnum _deactivation_scheme;

  /// Debug parameter - useful for coders, not for users (hopefully!)
  int _fspb_debug;

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

  /// plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// Old value of plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// internal parameters
  MaterialProperty<std::vector<Real> > & _intnl;

  /// old values of internal parameters
  MaterialProperty<std::vector<Real> > & _intnl_old;

  /// yield functions
  MaterialProperty<std::vector<Real> > & _yf;

  /// Number of Newton-Raphson iterations used in the return-map
  MaterialProperty<Real> & _iter;

  /// User objects that define the yield functions, flow potentials, etc
  std::vector<const TensorMechanicsPlasticModel *> _f;



  /**
   * The active yield function(s)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active yield functions are put into "f"
   * @param num_active number of active constraints
   * @param f (output) the yield function (or functions in the case of multisurface plasticity)
   */
  virtual void yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & f);

  /**
   * The derivative of the active yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "df_dstress"
   * @param num_active number of active constraints
   * @param df_dstress (output) the derivative (or derivatives in the case of multisurface plasticity).  df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & df_dstress);

  /**
   * The derivative of active yield function(s) with respect to their internal parameters (the user objects assume there is exactly one internal param per yield function)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "df_dintnl"
   * @param num_active number of active constraints
   * @param df_dintnl (output) the derivatives.  df_dstress[alpha] = dyieldFunction[alpha]/dintnl[alpha]
   */
  virtual void dyieldFunction_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & df_dintnl);

  /**
   * The active flow potential(s) - one for each yield function
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active flow potentials are put into "r"
   * @param num_active number of active constraints
   * @param r (output) the flow potential (flow potentials in the multi-surface case)
   */
  virtual void flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & r);

  /**
   * The derivative of the active flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dr_dstress"
   * @param num_active number of active constraints
   * @param dr_dstress (output) the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankFourTensor> & dr_dstress);

  /**
   * The derivative of the active flow potentials with respect to the active internal parameters
   * The UserObjects explicitly assume that r[alpha] is only dependent on intnl[alpha]
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dr_dintnl"
   * @param num_active number of active constraints
   * @param dr_dintnl (output) the derivatives.  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl[alpha]
   */
  virtual void dflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & dr_dintnl);

  /**
   * The active hardening potentials (one for each internal parameter and for each yield function)
   * by assumption in the Userobjects, the h[a][alpha] is nonzero only for a = alpha, so we only calculate those here
   * @param stress the stress at which to calculate the hardening potential
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active hardening potentials are put into "h"
   * @param num_active number of active constraints
   * @param h (output) the hardening potentials.  h[alpha] = hardening potential for yield fcn alpha and internal param a=alpha, by assumption in the userobjects this is only nonzero for a=alpha
   */
  virtual void hardPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & h);

  /**
   * The derivative of the active hardening potentials with respect to stress
   * By assumption in the Userobjects, the h[a][alpha] is nonzero only for a = alpha, so we only calculate those here
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dh_dstress"
   * @param num_active number of active constraints
   * @param dh_dstress (output) the derivative.  dh_dstress[a](i, j) = dh[a]/dstress(k, l)
   */
  virtual void dhardPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<RankTwoTensor> & dh_dstress);

  /**
   * The derivative of the active hardening potentials with respect to the active internal parameters
   * @param stress the stress at which to calculate the hardening potentials
   * @param intnl vector of internal parameters
   * @param active set of active constraints - only the active derivatives are put into "dh_dintnl"
   * @param num_active number of active constraints
   * @param dh_dintnl (output) the derivatives.  dh_dintnl[a][alpha][b] = dh[a][alpha]/dintnl[b].  Note that the userobjects assume that there is exactly one internal parameter per yield function, so the derivative is only nonzero for a=alpha=b, so that is all we calculate
   */
  virtual void dhardPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, unsigned num_active, std::vector<Real> & dh_dintnl);


  // **********************************************************************
  // *                                                                    *
  // *  USUALLY IT IS NOT NECESSARY TO OVERRIDE ANYTHING ELSE BELOW HERE  *
  // *                                                                    *
  // **********************************************************************

  /**
   * The active constraints.  These are set to zero (or <=0 in the case of the yield functions)
   * by the Newton-Raphson process
   * @param stress The stress
   * @param intnl_old old values of the internal parameters
   * @param intnl internal parameters
   * @param pm Current value(s) of the plasticity multiplier(s) (consistency parameters)
   * @param delta_dp Change in plastic strain incurred so far during the return
   * @param f (output) Active yield function(s)
   * @param epp (output) Plastic-strain increment constraint
   * @param ic (output) Active internal-parameter constraint
   * @param active The active constraints.  This is not modified if deactivate_if_linear_dependence = false, otherwise it may be modified
   * @param deactivate_if_linear_dependence The "deactivated_due_to_ld" vector may be modified if linear dependence in the return directions is detected
   * @param deactivated_due_to_ld This is not modified if deactivate_if_linear_dependence = false, otherwise it may be modified
   */
  virtual void calculateConstraints(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic, const std::vector<bool> & active, const bool & deactivate_if_linear_dependence, std::vector<bool> & deactivated_due_to_ld);

  /**
   * Performs a number of singular-value decompositions
   * to check for linear-dependence of the active directions "r"
   * If linear dependence is found, then f, r, active and num_active is modified appropriately
   * @param stress the current stress
   * @param intnl the current values of internal parameters
   * @param f (input/output) Upon output, these are the yield function values that are both active and not deactivated_due_to_ld
   * @param r (input) the flow directions that for those yield functions that are active upon entry to this function
   * @param active true if active
   * @param num_active number of active yield functions
   * @param (output) deactivated_due_to_ld Yield functions deactivated due to linearly-dependent flow directions
   */
  virtual void eliminateLinearDependence(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<Real> & f, const std::vector<RankTwoTensor> & r, const std::vector<bool> & active, unsigned num_active, std::vector<bool> & deactivated_due_to_ld);

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
   * makes all deactivated_due_to_ld false, and if >0 of them were initially true, returns true
   */
  virtual bool reinstateLinearDependentConstraints(std::vector<bool> & deactivated_due_to_ld);


  /**
   * counts the number of active constraints
   */
  virtual unsigned int numberActive(const std::vector<bool> & active);


  /**
   * Given the constraints, calculate the RHS which is
   * rhs = -(epp(0,0), epp(1,0), epp(1,1), epp(2,0), epp(2,1), epp(2,2), f[0], f[1], ..., f[num_f], ic[0], ic[1], ..., ic[num_ic])
   *
   * @param epp Plastic strain increment constraint
   * @param f yield function(s)
   * @param ic internal constraint(s)
   * @param rhs (output) the rhs
   * @param active the active constraints
   * @param deactivated_due_to_ld (output) constraints deactivated due to linear-dependence of flow directions
   */
  virtual void calculateRHS(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, std::vector<Real> & rhs, const std::vector<bool> & active, std::vector<bool> & deactivated_due_to_ld);

  /**
   * The residual-squared
   * @param pm the plastic multipliers for all constraints
   * @param f the active yield function(s) (not including the ones that are deactivated_due_to_ld)
   * @param epp the plastic strain increment constraint
   * @param ic the active internal constraint(s) (not including the ones that are deactivated_due_to_ld)
   * @param active true if constraint is active
   * @param deactivated_due_to_ld true if constraint has been temporarily deactivated due to linear dependence of flow directions
   */
  virtual Real residual2(const std::vector<Real> & pm, const std::vector<Real> & f, const RankTwoTensor & epp, const std::vector<Real> & ic, const std::vector<bool> & active, const std::vector<bool> & deactivated_due_to_ld);

  /**
   * d(rhs)/d(dof)
   */
  virtual void calculateJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankFourTensor & E_inv, const std::vector<bool> & active, const std::vector<bool> & deactivated_due_to_ld, std::vector<std::vector<Real> > & jac);

  /**
   * Implements the return map
   * @param stress_old  The stress at the previous "time" step
   * @param plastic_strain_old  The value of plastic strain at the previous "time" step
   * @param intnl_old The internal variables at the previous "time" step
   * @param delta_d  The total strain increment for this "time" step
   * @param E_ijkl   The elasticity tensor.  If no plasiticity then sig_new = sig_old + E_ijkl*delta_d
   * @param stress    (output) The stress after returning to the yield surface
   * @param plastic_strain   (output) The value of plastic strain after returning to the yield surface
   * @param intnl    (output) All the internal variables after returning to the yield surface
   * @param f  (output) All the yield functions after returning to the yield surface
   * @param iter (output) The number of Newton-Raphson iterations used
   * @return true if the stress was successfully returned to the yield surface
   * Note that this algorithm doesn't do any rotations.  In order to find the
   * final stress and plastic_strain must be rotated using _rotation_increment.
   * This is usually done in computeQpStress
   */
  virtual bool returnMap(const RankTwoTensor & stress_old, const RankTwoTensor & plastic_strain_old, const std::vector<Real> & intnl_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & stress, RankTwoTensor & plastic_strain, std::vector<Real> & intnl, std::vector<Real> & f, unsigned int & iter);

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

  /**
   * Performs a line search.  Algorithm is taken straight from
   * "Numerical Recipes".  Given the changes dstress, dpm and dintnl
   * provided by the nrStep routine, a line-search looks for an appropriate
   * under-relaxation that reduces the residual-squared (nr_res2).
   *
   * Most variables are input/output variables: they enter the function
   * with their values at the start of the Newton step, and they exit
   * the function with values attained after applying the under-relaxation
   *
   * @param nr_res2 (input/output) The residual-squared
   * @param intnl_old  The internal variables at the previous "time" step
   * @param intnl (input/output) The internal variables
   * @param pm (input/output) The plasticity multiplier(s) (consistency parameter(s))
   * @param E_inv inverse of the elasticity tensor
   * @param delta_dp (input/output) Change in plastic strain from start of "time" step to current configuration (plastic_strain - plastic_strain_old)
   * @param dstress Change in stress for a full Newton step
   * @param dpm Change in plasticity multiplier for a full Newton step
   * @param dintnl change in internal parameter(s) for a full Newton step
   * @param f (input/output) Yield function(s).  In this routine, only the active constraints that are not deactivated_due_to_ld are contained in f.
   * @param epp (input/output) Plastic strain increment constraint
   * @param ic (input/output) Internal constraint.  In this routine, only the active constraints that are not deactivated_due_to_ld are contained in ic.
   * @param active The active constraints.  This is not modified, but can't be made "const" because of how calculateConstraints is coded
   * @param deactivated_due_to_ld True if a constraint has temporarily been made deactive due to linear dependence.  This is not modified, but can't be made "const" because of how calculateConstraints is coded
   * @return true if successfully found a step that reduces the residual-squared
   */
  virtual bool lineSearch(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & pm, const RankFourTensor & E_inv, RankTwoTensor & delta_dp, const RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl, std::vector<Real> & f, RankTwoTensor & epp, std::vector<Real> & ic, std::vector<bool> & active, std::vector<bool> & deactivated_due_to_ld);


  /**
   * Performs a single Newton-Raphson + linesearch step
   * Constraints are deactivated and the step is re-done if
   * deactivation_scheme is set appropriately
   * @param nr_res2 (input/output) Residual-squared that the line-search will reduce
   * @param stress (input/output) stress
   * @param intnl_old (input) old values of the internal parameters
   * @param intnl (input/output) internal parameters
   * @param pm (input/output) plastic multipliers
   * @param delta_dp (input/output) Change in plastic strain from start of "time" step to current configuration (plastic_strain - plastic_strain_old)
   * @param E_inv (input) Inverse of the elasticity tensor
   * @param f (input/output) Yield function(s).  Upon successful exit only the active constraints are contained in f
   * @param epp (input/output) Plastic strain increment constraint
   * @param ic (input/output) Internal constraint.  Upon successful exit only the active constraints are contained in ic
   * @param active The active constraints.  This is may be modified, depending upon deactivation_scheme
   * @param deactivation_scheme The scheme used for deactivating constraints
   * @return true if the step was successful, ie, if the linesearch was successful and the number of constraints wasn't reduced to zero via deactivation
   */
  virtual bool singleStep(Real & nr_res2, RankTwoTensor & stress, const std::vector<Real> & intnl_old, std::vector<Real> & intnl, std::vector<Real> & pm, RankTwoTensor & delta_dp, const RankFourTensor & E_inv, std::vector<Real> & f,RankTwoTensor & epp, std::vector<Real> & ic, std::vector<bool> & active, const MooseEnum & deactivation_scheme);

  /**
   * Checks Kuhn-Tucker conditions, and alters "active" if appropriate.
   * Do not let the simplicity of this routine fool you!
   * Explicitly:
   * (1) checks that pm = 0 for all the f < 0.  If not, then active is set to false for that constraint.  This may be triggered if upon exit of the NR loops a constraint got deactivated due to linear dependence, and then f<0 and its pm>0.
   * (2) checks that pm = 0 for all inactive constraints.  This should always be true unless someone has screwed with the code.
   * (3) if any pm < 0, then active is set to false for that constraint.  This may be triggered if _deactivation_scheme!="optimized".
   * @param f values of the active yield functions
   * @param pm values of all the plastic multipliers
   * @param active the active constraints (true if active)
   * @param return false if any of the Kuhn-Tucker conditions were violated (and hence the set of active constraints was changed)
   */
  virtual bool checkAndApplyKuhnTucker(const std::vector<Real> & f, const std::vector<Real> & pm, std::vector<bool> & active);


 private:


  // Following are used for checking derivatives

  /**
   * Checks the derivatives, eg dyieldFunction_dstress by using
   * finite difference approximations.
   */
  void checkDerivatives();

  /**
   * The finite-difference derivative of yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param df_dstress (output) the derivative (or derivatives in the case of multisurface plasticity).  df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  void fddyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress);

  /**
   * The finite-difference derivative of the flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param dr_dstress (output) the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void fddflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankFourTensor> & dr_dstress);

  /**
   * The finite-difference derivative of the flow potentials with respect to internal parameters
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param dr_dintnl (output) the derivatives.  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl[alpha]
   */
  virtual void fddflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & dr_dintnl);

  /**
   * Checks the full Jacobian, which is just certain
   * linear combinations of the dyieldFunction_dstress, etc,
   * by using finite difference approximations
   */
  void checkJacobian();

  /**
   * The Jacobian calculated using finite differences.  The output
   * should be equal to calculateJacobian(...) if everything is
   * coded correctly.
   * @param stress the stress at which to calculate the Jacobian
   * @param intnl_old the old values of internal variables (jacobian is inependent of these, but they are needed to do the finite-differencing cleanly)
   * @param intnl the vector of internal parameters at which to calculate the Jacobian
   * @param pm the plasticity multipliers at which to calculate the Jacobian
   * @param delta_dp plastic_strain - plastic_strain_old (Jacobian is independent of this, but it is needed to do the finite-differencing cleanly)
   * @param E_inv inverse of the elasticity tensor
   * @param jac (output) the finite-difference Jacobian
   */
  virtual void fdJacobian(const RankTwoTensor & stress, const std::vector<Real> & intnl_old, const std::vector<Real> & intnl, const std::vector<Real> & pm, const RankTwoTensor & delta_dp, const RankFourTensor & E_inv, std::vector<std::vector<Real> > & jac);

  /**
   * Outputs the debug parameters: _fspb_debug_stress, _fspd_debug_pm, etc
   * and checks that they are sized correctly
   */
  void outputAndCheckDebugParameters();

};

#endif //FINITESTRAINMULTIPLASTICITY
