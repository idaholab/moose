//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeStressBase.h"
#include "MultiPlasticityDebugger.h"

/**
 * ComputeMultiPlasticityStress performs the return-map
 * algorithm and associated stress updates for plastic
 * models defined by a General User Objects
 *
 * Note that if run in debug mode you might have to use
 * the --no-trap-fpe flag because PETSc-LAPACK-BLAS
 * explicitly compute 0/0 and 1/0, and this causes
 * Libmesh to trap the floating-point exceptions
 */
class ComputeMultiPlasticityStress : public ComputeStressBase, public MultiPlasticityDebugger
{
public:
  static InputParameters validParams();

  ComputeMultiPlasticityStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  /// Maximum number of Newton-Raphson iterations allowed
  unsigned int _max_iter;

  /// Minimum fraction of applied strain that may be applied during adaptive stepsizing
  Real _min_stepsize;

  /// "dumb" deactivation will only be used if the stepsize falls below this quantity
  Real _max_stepsize_for_dumb;

  /// Even if the returnMap fails, return the best values found for stress and internal parameters
  bool _ignore_failures;

  /// The type of tangent operator to return.  tangent operator = d(stress_rate)/d(strain_rate).
  enum TangentOperatorEnum
  {
    elastic,
    linear,
    nonlinear
  } _tangent_operator_type;

  /// Tolerance on the plastic strain increment ("direction") constraint
  Real _epp_tol;

  /// dummy "consistency parameters" (plastic multipliers) used in quickStep when called from computeQpStress
  std::vector<Real> _dummy_pm;

  /**
   * the sum of the plastic multipliers over all the sub-steps.
   * This is used for calculating the consistent tangent operator
   */
  std::vector<Real> _cumulative_pm;

  /*
   * Scheme by which constraints are deactivated.
   * This enum is defined here for computational
   * efficiency.  If you add to this list you must
   * also add to the MooseEnum in the .C file
   */
  enum DeactivationSchemeEnum
  {
    optimized,
    safe,
    dumb,
    optimized_to_safe,
    safe_to_dumb,
    optimized_to_safe_to_dumb,
    optimized_to_dumb
  } _deactivation_scheme;

  /// User supplied the transverse direction vector
  bool _n_supplied;

  /// the supplied transverse direction vector
  RealVectorValue _n_input;

  /// rotation matrix that takes _n to (0, 0, 1)
  RealTensorValue _rot;

  /// whether to perform the rotations necessary in finite-strain simulations
  bool _perform_finite_strain_rotations;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// Old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// internal parameters
  MaterialProperty<std::vector<Real>> & _intnl;

  /// old values of internal parameters
  const MaterialProperty<std::vector<Real>> & _intnl_old;

  /// yield functions
  MaterialProperty<std::vector<Real>> & _yf;

  /// Number of Newton-Raphson iterations used in the return-map
  MaterialProperty<Real> & _iter;

  /// Whether a line-search was needed in the latest Newton-Raphson process (1 if true, 0 otherwise)
  MaterialProperty<Real> & _linesearch_needed;

  /// Whether linear-dependence was encountered in the latest Newton-Raphson process (1 if true, 0 otherwise)
  MaterialProperty<Real> & _ld_encountered;

  /// Whether constraints were added in during the latest Newton-Raphson process (1 if true, 0 otherwise)
  MaterialProperty<Real> & _constraints_added;

  /// current value of transverse direction
  MaterialProperty<RealVectorValue> & _n;

  /// old value of transverse direction
  const MaterialProperty<RealVectorValue> & _n_old;

  /// strain increment (coming from ComputeIncrementalStrain, for example)
  const MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Old value of total strain (coming from ComputeIncrementalStrain, for example)
  const MaterialProperty<RankTwoTensor> & _total_strain_old;

  /// Rotation increment (coming from ComputeIncrementalStrain, for example)
  const MaterialProperty<RankTwoTensor> & _rotation_increment;

  /// Old value of stress
  const MaterialProperty<RankTwoTensor> & _stress_old;

  /// Old value of elastic strain
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;

  /// whether Cosserat mechanics should be used
  bool _cosserat;

  /// The Cosserat curvature strain
  const MaterialProperty<RankTwoTensor> * const _curvature;

  /// The Cosserat elastic flexural rigidity tensor
  const MaterialProperty<RankFourTensor> * const _elastic_flexural_rigidity_tensor;

  /// the Cosserat couple-stress
  MaterialProperty<RankTwoTensor> * const _couple_stress;

  /// the old value of Cosserat couple-stress
  const MaterialProperty<RankTwoTensor> * const _couple_stress_old;

  /// derivative of couple-stress w.r.t. curvature
  MaterialProperty<RankFourTensor> * const _Jacobian_mult_couple;

  /// Elasticity tensor that can be rotated by this class (ie, its not const)
  RankFourTensor _my_elasticity_tensor;

  /// Strain increment that can be rotated by this class, and split into multiple increments (ie, its not const)
  RankTwoTensor _my_strain_increment;

  /// Flexual rigidity tensor that can be rotated by this class (ie, its not const)
  RankFourTensor _my_flexural_rigidity_tensor;

  /// Curvature that can be rotated by this class, and split into multiple increments (ie, its not const)
  RankTwoTensor _my_curvature;

  /**
   * makes all deactivated_due_to_ld false, and if >0 of them were initially true, returns true
   */
  virtual bool reinstateLinearDependentConstraints(std::vector<bool> & deactivated_due_to_ld);

  /**
   * counts the number of active constraints
   */
  virtual unsigned int numberActive(const std::vector<bool> & active);

  /**
   * The residual-squared
   * @param pm the plastic multipliers for all constraints
   * @param f the active yield function(s) (not including the ones that are deactivated_due_to_ld)
   * @param epp the plastic strain increment constraint
   * @param ic the active internal constraint(s) (not including the ones that are
   * deactivated_due_to_ld)
   * @param active true if constraint is active
   * @param deactivated_due_to_ld true if constraint has been temporarily deactivated due to linear
   * dependence of flow directions
   */
  virtual Real residual2(const std::vector<Real> & pm,
                         const std::vector<Real> & f,
                         const RankTwoTensor & epp,
                         const std::vector<Real> & ic,
                         const std::vector<bool> & active,
                         const std::vector<bool> & deactivated_due_to_ld);

  /**
   * Implements the return map
   *
   * Note that this algorithm doesn't do any rotations.  In order to find the
   * final stress and plastic_strain must be rotated using _rotation_increment.
   * This is usually done in computeQpStress
   *
   * @param stress_old The value of stress at the previous "time" step
   * @param[out] stress  The stress after returning to the yield surface
   * @param intnl_old The internal variables at the previous "time" step
   * @param[out] intnl     All the internal variables after returning to the yield surface
   * @param plastic_strain_old The value of plastic strain at the previous "time" step
   * @param[out] plastic_strain    The value of plastic strain after returning to the yield surface
   * @param E_ijkl   The elasticity tensor.  If no plasticity then stress = stress_old +
   * E_ijkl*strain_increment
   * @param strain_increment   The applied strain increment
   * @param[out] f   All the yield functions after returning to the yield surface
   * @param[out] iter  The number of Newton-Raphson iterations used
   * @param can_revert_to_dumb  If the _deactivation_scheme is set to revert to dumb, it will only
   * be allowed to do so if this parameter is true
   * @param[out] linesearch_needed  True if a linesearch was needed at any stage during the
   * Newton-Raphson proceedure
   * @param[out] ld_encountered  True if a linear-dependence of the flow directions was encountered
   * at any stage during the Newton-Raphson proceedure
   * @param[out] constraints_added  True if constraints were added into the active set at any stage
   * during the Newton-Raphson proceedure
   * @param final_step Each strain increment may be decomposed into a sum of smaller increments if
   * the return-map algorithm fails.  This flag indicates whether this is the last application of
   * incremental strain
   * @param[out] consistent_tangent_operator  The consistent tangent operator
   * d(stress_rate)/d(strain_rate).  This is only output if final_step=true, and the return value of
   * returnMap is also true.
   * @param[in,out] cumulative_pm Upon input: the plastic multipliers before the return map.  Upon
   * output: the plastic multipliers after this return map, if the return map was successful
   * @return true if the stress was successfully returned to the yield surface
   */
  virtual bool returnMap(const RankTwoTensor & stress_old,
                         RankTwoTensor & stress,
                         const std::vector<Real> & intnl_old,
                         std::vector<Real> & intnl,
                         const RankTwoTensor & plastic_strain_old,
                         RankTwoTensor & plastic_strain,
                         const RankFourTensor & E_ijkl,
                         const RankTwoTensor & strain_increment,
                         std::vector<Real> & f,
                         unsigned int & iter,
                         bool can_revert_to_dumb,
                         bool & linesearch_needed,
                         bool & ld_encountered,
                         bool & constraints_added,
                         bool final_step,
                         RankFourTensor & consistent_tangent_operator,
                         std::vector<Real> & cumulative_pm);

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
   * @param[in,out] nr_res2 The residual-squared
   * @param[out] stress  The stress after returning to the yield surface
   * @param intnl_old  The internal variables at the previous "time" step
   * @param[in,out] intnl The internal variables
   * @param[in,out] pm The plasticity multiplier(s) (consistency parameter(s))
   * @param E_inv inverse of the elasticity tensor
   * @param[in,out] delta_dp Change in plastic strain from start of "time" step to current
   * configuration (plastic_strain - plastic_strain_old)
   * @param dstress Change in stress for a full Newton step
   * @param dpm Change in plasticity multiplier for a full Newton step
   * @param dintnl change in internal parameter(s) for a full Newton step
   * @param[in,out] f Yield function(s).  In this routine, only the active constraints that are not
   * deactivated_due_to_ld are contained in f.
   * @param[in,out] epp Plastic strain increment constraint
   * @param[in,out] ic Internal constraint.  In this routine, only the active constraints that are
   * not deactivated_due_to_ld are contained in ic.
   * @param active The active constraints.
   * @param deactivated_due_to_ld True if a constraint has temporarily been made deactive due to
   * linear dependence.
   * @param[out] linesearch_needed  True if the full Newton-Raphson step was cut by the linesearch
   * @return true if successfully found a step that reduces the residual-squared
   */
  virtual bool lineSearch(Real & nr_res2,
                          RankTwoTensor & stress,
                          const std::vector<Real> & intnl_old,
                          std::vector<Real> & intnl,
                          std::vector<Real> & pm,
                          const RankFourTensor & E_inv,
                          RankTwoTensor & delta_dp,
                          const RankTwoTensor & dstress,
                          const std::vector<Real> & dpm,
                          const std::vector<Real> & dintnl,
                          std::vector<Real> & f,
                          RankTwoTensor & epp,
                          std::vector<Real> & ic,
                          const std::vector<bool> & active,
                          const std::vector<bool> & deactivated_due_to_ld,
                          bool & linesearch_needed);

  /**
   * Performs a single Newton-Raphson + linesearch step
   * Constraints are deactivated and the step is re-done if
   * deactivation_scheme is set appropriately
   * @param[in,out] nr_res2 Residual-squared that the line-search will reduce
   * @param[in,out] stress stress
   * @param[in] intnl_old old values of the internal parameters
   * @param[in,out] intnl internal parameters
   * @param[in,out] pm plastic multipliers
   * @param[in,out] delta_dp Change in plastic strain from start of "time" step to current
   * configuration (plastic_strain - plastic_strain_old)
   * @param[in] E_inv Inverse of the elasticity tensor
   * @param[in,out] f Yield function(s).  Upon successful exit only the active constraints are
   * contained in f
   * @param[in,out] epp Plastic strain increment constraint
   * @param[in,out] ic Internal constraint.  Upon successful exit only the active constraints are
   * contained in ic
   * @param active The active constraints.  This is may be modified, depending upon
   * deactivation_scheme
   * @param deactivation_scheme The scheme used for deactivating constraints
   * @param[out] linesearch_needed True if a linesearch was employed during this Newton-Raphson step
   * @param[out] ld_encountered True if a linear-dependence of the flow directions was encountered
   * at any stage during the Newton-Raphson proceedure
   * @return true if the step was successful, ie, if the linesearch was successful and the number of
   * constraints wasn't reduced to zero via deactivation
   */
  virtual bool singleStep(Real & nr_res2,
                          RankTwoTensor & stress,
                          const std::vector<Real> & intnl_old,
                          std::vector<Real> & intnl,
                          std::vector<Real> & pm,
                          RankTwoTensor & delta_dp,
                          const RankFourTensor & E_inv,
                          std::vector<Real> & f,
                          RankTwoTensor & epp,
                          std::vector<Real> & ic,
                          std::vector<bool> & active,
                          DeactivationSchemeEnum deactivation_scheme,
                          bool & linesearch_needed,
                          bool & ld_encountered);

  /**
   * Checks whether the yield functions are in the admissible region
   * @param stress stress
   * @param intnl internal parameters
   * @param[out] all_f the values of all the yield functions
   * @return return false if any yield functions exceed their tolerance
   */
  virtual bool checkAdmissible(const RankTwoTensor & stress,
                               const std::vector<Real> & intnl,
                               std::vector<Real> & all_f);

  /**
   * Builds the order which "dumb" activation will take.
   * @param stress stress to evaluate yield functions and derivatives at
   * @param intnl internal parameters to evaluate yield functions and derivatives at
   * @param[out] dumb_order dumb_order[0] will be the yield surface furthest away from (stress,
   * intnl), dumb_order[1] will be the next yield surface, etc.  The distance measure used is
   * f/|df_dstress|.  This array can then be fed into incrementDumb in order to first try the yield
   * surfaces which are farthest away from the (stress, intnl).
   */
  void buildDumbOrder(const RankTwoTensor & stress,
                      const std::vector<Real> & intnl,
                      std::vector<unsigned int> & dumb_order);

  /**
   * Increments "dumb_iteration" by 1, and sets "act" appropriately
   * (act[alpha] = true iff alpha_th bit of dumb_iteration == 1)
   * @param[in,out] dumb_iteration Used to set act bitwise - the "dumb" scheme tries all possible
   * combinations of act until a successful return
   * @param[in] dumb_order dumb_order dumb_order[0] will be the yield surface furthest away from
   * (stress, intnl), dumb_order[1] will be the next yield surface, etc.  The distance measure used
   * is f/|df_dstress|.  This array can then be fed into incrementDumb in order to first try the
   * yield surfaces which are farthest away from the (stress, intnl).
   * @param[out] act active constraints
   */
  virtual void incrementDumb(int & dumb_iteration,
                             const std::vector<unsigned int> & dumb_order,
                             std::vector<bool> & act);

  /**
   * Checks Kuhn-Tucker conditions, and alters "active" if appropriate.
   * Do not let the simplicity of this routine fool you!
   * Explicitly:
   * (1) checks that pm = 0 for all the f < 0.  If not, then active is set to false for that
   * constraint.  This may be triggered if upon exit of the NR loops a constraint got deactivated
   * due to linear dependence, and then f<0 and its pm>0.
   * (2) checks that pm = 0 for all inactive constraints.  This should always be true unless someone
   * has screwed with the code.
   * (3) if any pm < 0, then active is set to false for that constraint.  This may be triggered if
   * _deactivation_scheme!="optimized".
   * @param f values of the active yield functions
   * @param pm values of all the plastic multipliers
   * @param active the active constraints (true if active)
   * @return return false if any of the Kuhn-Tucker conditions were violated (and hence the set of
   * active constraints was changed)
   */
  virtual bool checkKuhnTucker(const std::vector<Real> & f,
                               const std::vector<Real> & pm,
                               const std::vector<bool> & active);

  /**
   * Checks Kuhn-Tucker conditions, and alters "active" if appropriate.
   * Do not let the simplicity of this routine fool you!
   * Explicitly:
   * (1) checks that pm = 0 for all the f < 0.  If not, then active is set to false for that
   * constraint.  This may be triggered if upon exit of the NR loops a constraint got deactivated
   * due to linear dependence, and then f<0 and its pm>0.
   * (2) checks that pm = 0 for all inactive constraints.  This should always be true unless someone
   * has screwed with the code.
   * (3) if any pm < 0, then active is set to false for that constraint.  This may be triggered if
   * _deactivation_scheme!="optimized".
   * @param f values of the active yield functions
   * @param pm values of all the plastic multipliers
   * @param active the active constraints (true if active)
   * @return return false if any of the Kuhn-Tucker conditions were violated (and hence the set of
   * active constraints was changed)
   */
  virtual void applyKuhnTucker(const std::vector<Real> & f,
                               const std::vector<Real> & pm,
                               std::vector<bool> & active);

  // gets called before any return-map
  virtual void preReturnMap();

  // gets called after return-map
  virtual void postReturnMap();

  /// The functions from which quickStep can be called
  enum quickStep_called_from_t
  {
    computeQpStress_function,
    returnMap_function
  };

  /**
   * Attempts to find an admissible (stress, intnl) by using the
   * customized return-map algorithms defined through the
   * SolidMechanicsPlasticXXXX.returnMap functions.
   *
   * @param stress_old The value of stress at the previous "time" step
   * @param[out] stress If returnvalue=true then this is the returned value of stress.  Otherwise,
   * this is undefined
   * @param intnl_old The internal variables at the previous "time" step
   * @param[out] intnl If returnvalue=true then this is the value of the internal parameters after
   * returning.  Otherwise, this is undefined
   * @param[out] pm If returnvalue=true, this is the plastic multipliers needed to bring aout the
   * return.  Otherwise, this is undefined
   * @param[in/out] cumulative_pm If returnvalue=true, this is cumulative plastic multipliers,
   * updated with pm.  Otherwise, this is untouched by the algorithm
   * @param plastic_strain_old The value of plastic strain at the previous "time" step
   * @param[out] plastic_strain  If returnvalue=true, this is the new plastic strain.  Otherwise it
   * is set to plastic_strain_old
   * @param E_ijkl   The elasticity tensor.
   * @param strain_increment   The applied strain increment
   * @param[out] yf If returnvalue=true, then all the yield functions at (stress, intnl).
   * Otherwise, all the yield functions at (stress_old, intnl_old)
   * @param[out] iterations Number of NR iterations used, which is always zero in the current
   * implementation.
   * @param called_from This can be called from computeQpStress, in which case it can actually
   * provde an answer to the returnmap algorithm, or from returnMap in which case it is probably
   * only providing an answer to a particular subdivision of the returnmap algorithm.  The
   * consistent tangent operator is calculated idfferently in each case
   * @param final_step  The consistent tangent operator is calculated if this is true
   * @param[out] consistent_tangent_operator If final_step==true and returnvalue=true, then this is
   * the consistent tangent operator d(stress_rate)/d(strain_rate).  Otherwise it is undefined.
   * @return true if the (stress, intnl) are admissible, in which case the (stress_old, intnl_old)
   * could have been admissible, or exactly one of the plastic models successfully used its custom
   * returnMap function to provide the returned (stress, intnl) values and all other plastic models
   * are admissible at that configuration.  Or, false, then (stress_old, intnl_old) is not
   * admissible according to >=1 plastic model and the custom returnMap functions failed in some
   * way.
   */
  virtual bool quickStep(const RankTwoTensor & stress_old,
                         RankTwoTensor & stress,
                         const std::vector<Real> & intnl_old,
                         std::vector<Real> & intnl,
                         std::vector<Real> & pm,
                         std::vector<Real> & cumulative_pm,
                         const RankTwoTensor & plastic_strain_old,
                         RankTwoTensor & plastic_strain,
                         const RankFourTensor & E_ijkl,
                         const RankTwoTensor & strain_increment,
                         std::vector<Real> & yf,
                         unsigned int & iterations,
                         RankFourTensor & consistent_tangent_operator,
                         const quickStep_called_from_t called_from,
                         bool final_step);

  /**
   * performs a plastic step
   *
   * @param stress_old The value of stress at the previous "time" step
   * @param[out] stress  stress after returning to the yield surface
   * @param intnl_old The internal variables at the previous "time" step
   * @param[out] intnl   internal variables after returning to the yield surface
   * @param plastic_strain_old The value of plastic strain at the previous "time" step
   * @param[out] plastic_strain    plastic_strain after returning to the yield surface
   * @param E_ijkl   The elasticity tensor.
   * @param strain_increment   The applied strain increment
   * @param[out] yf   All the yield functions at (stress, intnl)
   * @param[out] iterations  The total number of Newton-Raphson iterations used
   * @param[out] linesearch_needed  True if a linesearch was needed at any stage during the
   * Newton-Raphson proceedure
   * @param[out] ld_encountered  True if a linear-dependence of the flow directions was encountered
   * at any stage during the Newton-Raphson proceedure
   * @param[out] constraints_added  True if constraints were added into the active set at any stage
   * during the Newton-Raphson proceedure
   * @param[out] consistent_tangent_operator  The consistent tangent operator
   * d(stress_rate)/d(strain_rate)
   * @return true if the (stress, intnl) are admissible.  Otherwise, if _ignore_failures==true, the
   * output variables will be the best admissible ones found during the return-map.  Otherwise, if
   * _ignore_failures==false, this routine will perform some finite-diference checks and call
   * mooseError
   */
  virtual bool plasticStep(const RankTwoTensor & stress_old,
                           RankTwoTensor & stress,
                           const std::vector<Real> & intnl_old,
                           std::vector<Real> & intnl,
                           const RankTwoTensor & plastic_strain_old,
                           RankTwoTensor & plastic_strain,
                           const RankFourTensor & E_ijkl,
                           const RankTwoTensor & strain_increment,
                           std::vector<Real> & yf,
                           unsigned int & iterations,
                           bool & linesearch_needed,
                           bool & ld_encountered,
                           bool & constraints_added,
                           RankFourTensor & consistent_tangent_operator);

  //  bool checkAndModifyConstraints(bool nr_exit_condition, const RankTwoTensor & stress, const
  //  std::vector<Real> & intnl, const std::vector<Real> & pm, const std::vector<bool> &
  //  initial_act, bool can_revert_to_dumb, const RankTwoTensor & initial_stress, const
  //  std::vector<Real> & intnl_old, const std::vector<Real> & f, DeactivationSchemeEnum
  //  deact_scheme, std::vector<bool> & act, int & dumb_iteration, std::vector<unsigned int>
  //  dumb_order, bool & die);

  bool canChangeScheme(DeactivationSchemeEnum current_deactivation_scheme, bool can_revert_to_dumb);

  bool canIncrementDumb(int dumb_iteration);

  void changeScheme(const std::vector<bool> & initial_act,
                    bool can_revert_to_dumb,
                    const RankTwoTensor & initial_stress,
                    const std::vector<Real> & intnl_old,
                    DeactivationSchemeEnum & current_deactivation_scheme,
                    std::vector<bool> & act,
                    int & dumb_iteration,
                    std::vector<unsigned int> & dumb_order);

  bool canAddConstraints(const std::vector<bool> & act, const std::vector<Real> & all_f);

  unsigned int activeCombinationNumber(const std::vector<bool> & act);

  /**
   * Computes the consistent tangent operator
   * (another name for the jacobian = d(stress_rate)/d(strain_rate)
   *
   * The computations performed depend upon _tangent_operator_type
   *
   * @param stress The value of stress after the return map algorithm has converged
   * @param intnl The internal parameters after the return map has converged
   * @param E_ijkl The elasticity tensor (in the case of no plasticity this is the jacobian)
   * @param pm_this_step The plastic multipliers coming from the final strain increment.  In many
   * cases these will be equal to cumulative_pm, but in the case where the returnMap algorithm had
   * to be performed in multiple substeps of smaller applied strain increments, pm_this_step are
   * just the plastic multipliers for the final application of the strain incrment
   * @param cumulative_pm The plastic multipliers needed for this current Return (this is the sum of
   * the plastic multipliers over all substeps if the strain increment was applied in small
   * substeps)
   */
  RankFourTensor consistentTangentOperator(const RankTwoTensor & stress,
                                           const std::vector<Real> & intnl,
                                           const RankFourTensor & E_ijkl,
                                           const std::vector<Real> & pm_this_step,
                                           const std::vector<Real> & cumulative_pm);

private:
  RankTwoTensor rot(const RankTwoTensor & tens);
};
