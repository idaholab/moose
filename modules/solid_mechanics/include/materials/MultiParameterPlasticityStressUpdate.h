//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StressUpdateBase.h"

#include <array>

/**
 * MultiParameterPlasticityStressUpdate performs the return-map
 * algorithm and associated stress updates for plastic
 * models where the yield function and flow directions
 * depend on multiple parameters (called "stress_params" in the
 * documentation and sp in the code) that
 * are themselves functions of stress.
 *
 * Let the stress_params be S = {S[0], S[1], ... S[N-1]}
 * and define _num_sp = N.
 *
 * For instance, CappedDruckerPrager plasticity has _num_sp = 2
 * and defines
 * S[0] = p = stress.trace()
 * S[1] = q = sqrt(stress.secondInvariant())
 *
 * The point of the stress_params is to reduce the number of
 * degrees of freedom involved in the return-map algorithm
 * (for computational efficiency as all the intensive computation
 * occurs in "stress_param space") and to allow users to
 * write their yield functions, etc, in forms that are
 * clear to them to avoid errors.
 *
 * Derived classes can describe plasticity via multiple yield
 * functions.  The number of yield function is _num_yf.
 * The yield function(s) and flow potential(s)
 * must be functions of the stress_params.
 *
 * Derived classes can use any number of internal parameters.
 * The number of internal parameters is _num_intnl.  The
 * derived classes must define the evolution of these internal
 * parameters, which are typically functions of certain
 * "distances" in the return-mapping procedure.
 *
 * For instance, CappedDruckerPrager plasticity has three
 * yield functions (a smoothed DruckerPrager cone, a tensile failure
 * envelope, and a compressive failure envelope) and two
 * internal parameters (plastic shear strain and plastic tensile strain).
 * The strength parameters (cohesion, friction angle,
 * dilation angle, tensile strength, compressive strength)
 * are functions of these internal parameters.
 *
 * A novel smoothing procedure is used to smooth the derived class's
 * yield functions into a single, smooth yield function.  The
 * smoothing is also performed for the flow potential.  All
 * return-mapping, etc, processes are performed on this single
 * yield function and flow potential.
 *
 * The return-map algorithm implements the updateState method of
 * StressUpdateBase.  In particular, the following system of
 * equations is solved:
 * 0 = _rhs[0] = S[0] - S[0]^trial + ga * E[0, j] * dg/dS[j]
 * 0 = _rhs[1] = S[1] - S[1]^trial + ga * E[1, j] * dg/dS[j]
 * ...
 * 0 = _rhs[N-1] = S[N-1] - S[N-1]^trial + ga * E[N-1, j] * dg/dS[j]
 * 0 = _rhs[N] = f(S, intnl)
 * (here there is an implied sum over j)
 * for the _num_sp stress_params S, and the single scalar ga
 * (actually I solve for gaE = ga * _En, where _En is a normalising
 * factor so that gaE is of similar magnitude to the S variables).
 * In general E[a, b] = dS[a]/dstress(i, j) E(i, j, k, l) dS[b]/dstress(k, l),
 * and in the code below it is assumed that the E[a, b] are independent
 * of the stress_params, but adding a dependence would only require
 * some Jacobian terms to be modified.
 * There are N+1 rhs equations to solve.  Here f is
 * the smoothed yield function, so the last equation is the admissibility
 * condition (the returned stress lies on the yield surface) and g
 * is the flow potential so the other conditions implement the normality
 * condition.  It is up to the derived classes to defined E[i, j]
 * so that the rhs really represents the normality condition.
 *
 * For instance, CappedDruckerPrager has (with Eijkl being the elasticity
 * tensor):
 * E[0, 0] = _Epp = Eijkl.sum3x3()
 * E[0, 1] = 0
 * E[1, 0] = 0
 * E[1, 1] = Eijkl(0, 1, 0, 1)
 */
class MultiParameterPlasticityStressUpdate : public StressUpdateBase
{
public:
  static InputParameters validParams();

  MultiParameterPlasticityStressUpdate(const InputParameters & parameters,
                                       unsigned num_sp,
                                       unsigned num_yf,
                                       unsigned num_intnl);

protected:
  virtual void initQpStatefulProperties() override;
  using StressUpdateBase::updateState;
  virtual void updateState(RankTwoTensor & strain_increment,
                           RankTwoTensor & inelastic_strain_increment,
                           const RankTwoTensor & rotation_increment,
                           RankTwoTensor & stress_new,
                           const RankTwoTensor & stress_old,
                           const RankFourTensor & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old,
                           bool compute_full_tangent_operator,
                           RankFourTensor & tangent_operator) override;

  virtual void propagateQpStatefulProperties() override;

  virtual TangentCalculationMethod getTangentCalculationMethod() override
  {
    return TangentCalculationMethod::FULL;
  }

  /// Internal dimensionality of tensors (currently this is 3 throughout tensor_mechanics)
  constexpr static unsigned _tensor_dimensionality = 3;

  /// Number of stress parameters
  const unsigned _num_sp;

  /// An admissible value of stress_params at the initial time
  const std::vector<Real> _definitely_ok_sp;

  /// E[i, j] in the system of equations to be solved
  std::vector<std::vector<Real>> _Eij;

  /// normalising factor
  Real _En;

  /// _Cij[i, j] * _Eij[j, k] = 1 iff j == k
  std::vector<std::vector<Real>> _Cij;

  /// Number of yield functions
  const unsigned _num_yf;

  /// Number of internal parameters
  const unsigned _num_intnl;

  /// Maximum number of Newton-Raphson iterations allowed in the return-map process
  const unsigned _max_nr_its;

  /// Whether to perform finite-strain rotations
  const bool _perform_finite_strain_rotations;

  /// Smoothing tolerance: edges of the yield surface get smoothed by this amount
  const Real _smoothing_tol;

  /// Square of the smoothing tolerance
  const Real _smoothing_tol2;

  /// The yield-function tolerance
  const Real _f_tol;

  /// Square of the yield-function tolerance
  const Real _f_tol2;

  /**
   * In order to help the Newton-Raphson procedure, the applied
   * strain increment may be applied in sub-increments of size
   * greater than this value.
   */
  const Real _min_step_size;

  /// handles case of initial_stress that is inadmissible being supplied
  bool _step_one;

  /// Output a warning message if precision loss is encountered during the return-map process
  const bool _warn_about_precision_loss;

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

  /// Maximum number of Newton-Raphson iterations used in the return-map during the course of the entire simulation
  MaterialProperty<Real> & _max_iter_used;

  /// Old value of maximum number of Newton-Raphson iterations used in the return-map during the course of the entire simulation
  const MaterialProperty<Real> & _max_iter_used_old;

  /// Whether a line-search was needed in the latest Newton-Raphson process (1 if true, 0 otherwise)
  MaterialProperty<Real> & _linesearch_needed;

  /**
   * Struct designed to hold info about a single yield function
   * and its derivatives, as well as the flow directions
   */
  struct yieldAndFlow
  {
    Real f;                                // yield function value
    std::vector<Real> df;                  // df/d(stress_param[i])
    std::vector<Real> df_di;               // df/d(intnl_variable[a])
    std::vector<Real> dg;                  // d(flow)/d(stress_param[i])
    std::vector<std::vector<Real>> d2g;    // d^2(flow)/d(sp[i])/d(sp[j])
    std::vector<std::vector<Real>> d2g_di; // d^2(flow)/d(sp[i])/d(intnl[a])

    yieldAndFlow() : yieldAndFlow(0, 0) {}

    yieldAndFlow(unsigned num_var, unsigned num_intnl)
      : f(0.0),
        df(num_var),
        df_di(num_intnl),
        dg(num_var),
        d2g(num_var, std::vector<Real>(num_var, 0.0)),
        d2g_di(num_var, std::vector<Real>(num_intnl, 0.0))
    {
    }

    // this may be involved in the smoothing of a group of yield functions
    bool operator<(const yieldAndFlow & fd) const { return f < fd.f; }
  };

  /**
   * Computes the smoothed yield function
   * @param stress_params The stress parameters (eg stress_params[0] = stress_zz and
   * stress_params[1] = sqrt(stress_zx^2 + stress_zy^2))
   * @param intnl The internal parameters (eg intnl[0] is shear, intnl[1] is tensile)
   * @return The smoothed yield function value
   */
  Real yieldF(const std::vector<Real> & stress_params, const std::vector<Real> & intnl) const;

  /**
   * Computes the smoothed yield function
   * @param yfs The values of the individual yield functions
   * @return The smoothed yield function value
   */
  Real yieldF(const std::vector<Real> & yfs) const;

  /**
   * Smooths yield functions.  The returned value must be zero if abs(f_diff) >= _smoothing_tol
   * and otherwise must satisfy, over -_smoothing_tol <= f_diff <= _smoothing_tol:
   * (1) C2
   * (2) zero at f_diff = +/- _smoothing_tol
   * (3) derivative is +/-0.5 at f_diff = +/- _smoothing_tol
   * (4) derivative must be in [-0.5, 0.5]
   * (5) second derivative is zero at f_diff = +/- _smoothing_tol
   * (6) second derivative must be non-negative
   * in order to ensure C2 differentiability and convexity of the smoothed yield surface.
   */
  Real ismoother(Real f_diff) const;

  /**
   * Derivative of ismoother
   */
  Real smoother(Real f_diff) const;

  /**
   * Derivative of smoother
   */
  Real dsmoother(Real f_diff) const;

  /**
   * Calculates all yield functions and derivatives, and then performs the smoothing scheme
   * @param stress_params[in] The stress parameters (eg stress_params[0] = stress_zz and
   * stress_params[1] = sqrt(stress_zx^2 + stress_zy^2))
   * @param intnl[in] Internal parameters
   * @return The smoothed yield function and derivatives
   */
  yieldAndFlow smoothAllQuantities(const std::vector<Real> & stress_params,
                                   const std::vector<Real> & intnl) const;

  /**
   * Performs a line-search to find stress_params and gaE
   * Upon entry:
   *  - rhs contains the *solution* to the Newton-Raphson (ie nrStep should have been called).  If a
   * full
   *    Newton step is used then stress_params[:] += rhs[0:_num_sp-1] and gaE += rhs[_num_sp]
   *  - res2 contains the residual-squared before applying any of solution
   *  - stress_params contains the stress_params before applying any of the solution
   *  - gaE contains gaE before applying any of the solution (that is contained in rhs)
   * Upon exit:
   *  - stress_params will be the stress_params after applying the solution
   *  - gaE will be the stress_params after applying the solution
   *  - rhs will contain the updated rhs values (after applying the solution) ready for the next
   * Newton-Raphson step,
   *  - res2 will be the residual-squared after applying the solution
   *  - intnl will contain the internal variables corresponding to the return from
   * trial_stress_params to stress_params
   *    (and starting from intnl_ok)
   *  - linesearch_needed will be 1.0 if a linesearch was needed
   *  - smoothed_q will contain the value of the yield function and its derivatives, etc, at
   * (stress_params, intnl)
   * @param res2[in,out] the residual-squared, both as an input and output
   * @param stress_params[in,out] Upon input the value of the stress_params before the current
   * Newton-Raphson process was initiated.
   * Upon exit this will hold the values coming from the line search.
   * @param trial_stress_params[in] Trial value for the stress_params for this (sub)strain increment
   * @param gaE[in,out] Upon input the value of gaE before the current Newton-Raphson iteration was
   * initiated.  Upon exit this will hold
   * the value coming from the line-search
   * @param smoothed_q[in,out] Upon input, the value of the smoothed yield function and derivatives
   * at the
   * prior-to-Newton configuration.  Upon exit this is evaluated at the new (stress_params, intnl)
   * @param intnl_ok[in] The value of the internal parameters from the start of this (sub)strain
   * increment
   * @param intnl[in,out] The value of the internal parameters after the line-search has converged
   * @param rhs[in,out] Upon entry this contains the solution to the Newton-Raphson.  Upon exit this
   * contains the updated rhs values
   * @return 0 if successful, 1 otherwise
   */
  int lineSearch(Real & res2,
                 std::vector<Real> & stress_params,
                 Real & gaE,
                 const std::vector<Real> & trial_stress_params,
                 yieldAndFlow & smoothed_q,
                 const std::vector<Real> & intnl_ok,
                 std::vector<Real> & intnl,
                 std::vector<Real> & rhs,
                 Real & linesearch_needed) const;

  /**
   * Performs a Newton-Raphson step to attempt to zero rhs
   * Upon return, rhs will contain the solution.
   * @param smoothed_q[in] The value of the smoothed yield function and derivatives prior to this
   * Newton-Raphson step
   * @param trial_stress_params[in] Trial value for the stress_params for this (sub)strain increment
   * @param stress_params[in] The current value of the stress_params
   * @param intnl[in] The current value of the internal parameters
   * @param gaE[in] The current value of gaE
   * @param rhs[in,out] Upon entry, the rhs to zero using Newton-Raphson.  Upon exit, the solution
   * to the Newton-Raphson problem
   * @return 0 if successful, 1 otherwise
   */
  int nrStep(const yieldAndFlow & smoothed_q,
             const std::vector<Real> & trial_stress_params,
             const std::vector<Real> & stress_params,
             const std::vector<Real> & intnl,
             Real gaE,
             std::vector<Real> & rhs) const;

  /**
   * Calculates the residual-squared for the Newton-Raphson + line-search
   * @param rhs[in] The RHS vector
   * @return sum_i (rhs[i] * rhs[i])
   */
  Real calculateRes2(const std::vector<Real> & rhs) const;

  /**
   * Calculates the RHS in the following
   * 0 = rhs[0] = S[0] - S[0]^trial + ga * E[0, j] * dg/dS[j]
   * 0 = rhs[1] = S[1] - S[1]^trial + ga * E[1, j] * dg/dS[j]
   * ...
   * 0 = rhs[N-1] = S[N-1] - S[N-1]^trial + ga * E[N-1, j] * dg/dS[j]
   * 0 = rhs[N] = f(S, intnl)
   * where N = _num_sp
   * @param trial_stress_params[in] The trial stress parameters for this (sub)strain increment,
   * S[:]^trial
   * @param stress_params[in] The current stress parameters, S[:]
   * @param gaE[in] ga*_En (the normalisation with _En is so that gaE is of similar magnitude to S)
   * @param smoothed_q[in] Holds the current value of yield function and derivatives evaluated at
   * the current stress parameters and the current internal parameters
   * @param rhs[out] The result
   */
  void calculateRHS(const std::vector<Real> & trial_stress_params,
                    const std::vector<Real> & stress_params,
                    Real gaE,
                    const yieldAndFlow & smoothed_q,
                    std::vector<Real> & rhs) const;

  /**
   * Derivative of -RHS with respect to the stress_params and gaE, placed
   * into an array ready for solving the linear system using
   * LAPACK gsev
   * @param smoothed_q[in] Holds the current value of yield function and derivatives evaluated at
   * the
   * current values of the stress_params and the internal parameters
   * @param dintnl[in] The derivatives of the internal parameters wrt the stress_params
   * @param stress_params[in] The current value of the stress_params during the Newton-Raphson
   * process
   * @param gaE[in] The current value of gaE
   * @param jac[out] The outputted derivatives
   */
  void dnRHSdVar(const yieldAndFlow & smoothed_q,
                 const std::vector<std::vector<Real>> & dintnl,
                 const std::vector<Real> & stress_params,
                 Real gaE,
                 std::vector<double> & jac) const;

  /**
   * Performs any necessary cleaning-up, then throw MooseException(message)
   * @param message The message to using in MooseException
   */
  virtual void errorHandler(const std::string & message) const;

  /**
   * Computes the values of the yield functions, given stress_params and intnl parameters.
   * Derived classes must override this, to provide the values of the yield functions
   * in yf.
   * @param stress_params[in] The stress parameters
   * @param intnl[in] The internal parameters
   * @param[out] yf The yield function values
   */
  virtual void yieldFunctionValuesV(const std::vector<Real> & stress_params,
                                    const std::vector<Real> & intnl,
                                    std::vector<Real> & yf) const = 0;

  /**
   * Completely fills all_q with correct values.  These values are:
   * (1) the yield function values, yf[i]
   * (2) d(yf[i])/d(stress_params[j])
   * (3) d(yf[i])/d(intnl[j])
   * (4) d(flowPotential[i])/d(stress_params[j])
   * (5) d2(flowPotential[i])/d(stress_params[j])/d(stress_params[k])
   * (6) d2(flowPotential[i])/d(stress_params[j])/d(intnl[k])
   * @param stress_params[in] The stress parameters
   * @param intnl[in] The internal parameters
   * @param[out] all_q All the desired quantities
   */
  virtual void computeAllQV(const std::vector<Real> & stress_params,
                            const std::vector<Real> & intnl,
                            std::vector<yieldAndFlow> & all_q) const = 0;

  /**
   * Derived classes may employ this function to record stuff or do
   * other computations prior to the return-mapping algorithm.  We
   * know that (trial_stress_params, intnl_old) is inadmissible when
   * this is called
   * @param trial_stress_params[in] The trial values of the stress parameters
   * @param stress_trial[in] Trial stress tensor
   * @param intnl_old[in] Old value of the internal parameters.
   * @param yf[in] The yield functions at (p_trial, q_trial, intnl_old)
   * @param Eijkl[in] The elasticity tensor
   */
  virtual void preReturnMapV(const std::vector<Real> & trial_stress_params,
                             const RankTwoTensor & stress_trial,
                             const std::vector<Real> & intnl_old,
                             const std::vector<Real> & yf,
                             const RankFourTensor & Eijkl);

  /**
   * Sets (stress_params, intnl) at "good guesses" of the solution to the Return-Map algorithm.
   * The purpose of these "good guesses" is to speed up the Newton-Raphson process by providing
   * it with a good initial guess.
   * Derived classes may choose to override this if their plastic models are easy
   * enough to solve approximately.
   * The default values, provided by this class, are simply gaE = 0, stress_params =
   * trial_stress_params,
   * that is, the "good guess" is just the trial point for this (sub)strain increment.
   * @param trial_stress_params[in] The stress_params at the trial point
   * @param intnl_old[in] The internal parameters before applying the (sub)strain increment
   * @param stress_params[out] The "good guess" value of the stress_params
   * @param gaE[out] The "good guess" value of gaE
   * @param intnl[out] The "good guess" value of the internal parameters
   */
  virtual void initializeVarsV(const std::vector<Real> & trial_stress_params,
                               const std::vector<Real> & intnl_old,
                               std::vector<Real> & stress_params,
                               Real & gaE,
                               std::vector<Real> & intnl) const;

  /**
   * Sets the internal parameters based on the trial values of
   * stress_params, their current values, and the old values of the
   * internal parameters.
   * Derived classes must override this.
   * @param trial_stress_params[in] The trial stress parameters (eg trial_p and trial_q)
   * @param current_stress_params[in] The current stress parameters (eg p and q)
   * @param intnl_old[out] Old value of internal parameters
   * @param intnl[out] The value of internal parameters to be set
   */
  virtual void setIntnlValuesV(const std::vector<Real> & trial_stress_params,
                               const std::vector<Real> & current_stress_params,
                               const std::vector<Real> & intnl_old,
                               std::vector<Real> & intnl) const = 0;

  /**
   * Sets the derivatives of internal parameters, based on the trial values of
   * stress_params, their current values, and the current values of the
   * internal parameters.
   * Derived classes must override this.
   * @param trial_stress_params[in] The trial stress parameters
   * @param current_stress_params[in] The current stress parameters
   * @param intnl[in] The current value of the internal parameters
   * @param dintnl[out] The derivatives dintnl[i][j] = d(intnl[i])/d(stress_param j)
   */
  virtual void setIntnlDerivativesV(const std::vector<Real> & trial_stress_params,
                                    const std::vector<Real> & current_stress_params,
                                    const std::vector<Real> & intnl,
                                    std::vector<std::vector<Real>> & dintnl) const = 0;

  /**
   * Computes stress_params, given stress.  Derived classes must
   * override this
   * @param stress[in] Stress tensor
   * @param stress_params[out] The compute stress_params
   */
  virtual void computeStressParams(const RankTwoTensor & stress,
                                   std::vector<Real> & stress_params) const = 0;

  /**
   * Derived classes may use this to perform calculations before
   * any return-map process is performed, for instance, to initialize
   * variables.
   * This is called at the very start of updateState, even before
   * any checking for admissible stresses, etc, is performed
   */
  virtual void initializeReturnProcess();

  /**
   * Derived classes may use this to perform calculations after the
   * return-map process has completed successfully in stress_param space
   * but before the returned stress tensor has been calculcated.
   * @param rotation_increment[in] The large-strain rotation increment
   */
  virtual void finalizeReturnProcess(const RankTwoTensor & rotation_increment);

  /**
   * Sets stress from the admissible parameters.
   * This is called after the return-map process has completed
   * successfully in stress_param space, just after finalizeReturnProcess
   * has been called.
   * Derived classes must override this function
   * @param stress_trial[in] The trial value of stress
   * @param stress_params[in] The value of the stress_params after the return-map process has
   * completed successfully
   * @param gaE[in] The value of gaE after the return-map process has completed successfully
   * @param intnl[in] The value of the internal parameters after the return-map process has
   * completed successfully
   * @param smoothed_q[in] Holds the current value of yield function and derivatives evaluated at
   * the returned state
   * @param Eijkl[in] The elasticity tensor
   * @param stress[out] The returned value of the stress tensor
   */
  virtual void setStressAfterReturnV(const RankTwoTensor & stress_trial,
                                     const std::vector<Real> & stress_params,
                                     Real gaE,
                                     const std::vector<Real> & intnl,
                                     const yieldAndFlow & smoothed_q,
                                     const RankFourTensor & Eijkl,
                                     RankTwoTensor & stress) const = 0;

  /**
   * Sets inelastic strain increment from the returned configuration
   * This is called after the return-map process has completed
   * successfully in stress_param space, just after finalizeReturnProcess
   * has been called.
   * Derived classes may override this function
   * @param stress_trial[in] The trial value of stress
   * @param gaE[in] The value of gaE after the return-map process has completed successfully
   * @param smoothed_q[in] Holds the current value of yield function and derivatives evaluated at
   * the returned configuration
   * @param elasticity_tensor[in] The elasticity tensor
   * @param returned_stress[in] The stress after the return-map process
   * @param inelastic_strain_increment[out] The inelastic strain increment resulting from this
   * return-map
   */
  virtual void
  setInelasticStrainIncrementAfterReturn(const RankTwoTensor & stress_trial,
                                         Real gaE,
                                         const yieldAndFlow & smoothed_q,
                                         const RankFourTensor & elasticity_tensor,
                                         const RankTwoTensor & returned_stress,
                                         RankTwoTensor & inelastic_strain_increment) const;

  /**
   * Calculates the consistent tangent operator.
   * Derived classes may choose to override this for computational
   * efficiency.  The implementation in this class is quite expensive,
   * even though it looks compact and clean, because of all the
   * manipulations of RankFourTensors involved.
   * @param stress_trial[in] the trial value of the stress tensor for this strain increment
   * @param trial_stress_params[in] the trial values of the stress_params for this strain increment
   * @param stress[in] the returned value of the stress tensor for this strain increment
   * @param stress_params[in] the returned value of the stress_params for this strain increment
   * @param gaE[in] the total value of that came from this strain increment
   * @param smoothed_q[in] contains the yield function and derivatives evaluated at (p, q)
   * @param Eijkl[in] The elasticity tensor
   * @param compute_full_tangent_operator[in] true if the full consistent tangent operator is
   * needed,
   * otherwise false
   * @param dvar_dtrial[in] dvar_dtrial[i][j] = d({stress_param[i],gaE})/d(trial_stress_param[j])
   * for
   * this strain increment
   * @param[out] cto The consistent tangent operator
   */
  virtual void consistentTangentOperatorV(const RankTwoTensor & stress_trial,
                                          const std::vector<Real> & trial_stress_params,
                                          const RankTwoTensor & stress,
                                          const std::vector<Real> & stress_params,
                                          Real gaE,
                                          const yieldAndFlow & smoothed_q,
                                          const RankFourTensor & Eijkl,
                                          bool compute_full_tangent_operator,
                                          const std::vector<std::vector<Real>> & dvar_dtrial,
                                          RankFourTensor & cto);

  /**
   * d(stress_param[i])/d(stress) at given stress
   * @param stress stress tensor
   * @return d(stress_param[:])/d(stress)
   */
  virtual std::vector<RankTwoTensor> dstress_param_dstress(const RankTwoTensor & stress) const = 0;

  /**
   * d2(stress_param[i])/d(stress)/d(stress) at given stress
   * @param stress stress tensor
   * @return d2(stress_param[:])/d(stress)/d(stress)
   */
  virtual std::vector<RankFourTensor>
  d2stress_param_dstress(const RankTwoTensor & stress) const = 0;

  /// Sets _Eij and _En and _Cij
  virtual void setEffectiveElasticity(const RankFourTensor & Eijkl) = 0;

  /**
   * Calculates derivatives of the stress_params and gaE with repect to the
   * trial values of the stress_params for the (sub)strain increment.
   * After the strain increment has been fully applied, dvar_dtrial will
   * contain the result appropriate to the full strain increment.  Before
   * that time (if applying in sub-strain increments) it will contain the
   * result appropriate to the amount of strain increment applied successfully.
   * @param elastic_only[in] whether this was an elastic step: if so then the updates to dvar_dtrial
   * are fairly trivial
   * @param trial_stress_params[in] Trial values of stress_params for this (sub)strain increment
   * @param stress_params[in] Returned values of stress_params for this (sub)strain increment
   * @param gaE[in] the value of gaE that came from this (sub)strain increment
   * @param intnl[in] the value of the internal parameters at the returned position
   * @param smoothed_q[in] contains the yield function and derivatives evaluated at (stress_params,
   * intnl)
   * @param step_size[in] size of this (sub)strain increment
   * @param compute_full_tangent_operator[in] true if the full consistent tangent operator is
   * needed,
   * otherwise false
   * @param dvar_dtrial[out] dvar_dtrial[i][j] = d({stress_param[i],gaE})/d(trial_stress_param[j])
   */
  void dVardTrial(bool elastic_only,
                  const std::vector<Real> & trial_stress_params,
                  const std::vector<Real> & stress_params,
                  Real gaE,
                  const std::vector<Real> & intnl,
                  const yieldAndFlow & smoothed_q,
                  Real step_size,
                  bool compute_full_tangent_operator,
                  std::vector<std::vector<Real>> & dvar_dtrial) const;

  /**
   * Check whether precision loss has occurred
   * @param[in] solution The solution to the Newton-Raphson system
   * @param[in] stress_params The currect values of the stress_params for this (sub)strain increment
   * @param[in] gaE The currenct value of gaE for this (sub)strain increment
   * @return true if precision loss has occurred
   */
  bool precisionLoss(const std::vector<Real> & solution,
                     const std::vector<Real> & stress_params,
                     Real gaE) const;

private:
  /**
   * "Trial" value of stress_params that initializes the return-map process
   * This is derived from stress = stress_old + Eijkl * strain_increment.
   * However, since the return-map process can fail and be restarted by
   * applying strain_increment in multiple substeps, _trial_sp can vary
   * from substep to substep.
   */
  std::vector<Real> _trial_sp;

  /**
   * "Trial" value of stress that is set at the beginning of the
   * return-map process.  It is fixed at stress_old + Eijkl * strain_increment
   * irrespective of any sub-stepping
   */
  RankTwoTensor _stress_trial;

  /**
   * 0 = rhs[0] = S[0] - S[0]^trial + ga * E[0, i] * dg/dS[i]
   * 0 = rhs[1] = S[1] - S[1]^trial + ga * E[1, i] * dg/dS[i]
   * ...
   * 0 = rhs[N-1] = S[N-1] - S[N-1]^trial + ga * E[N-1, i] * dg/dS[i]
   * 0 = rhs[N] = f(S, intnl)
   * Here N = num_sp
   */
  std::vector<Real> _rhs;

  /**
   * d({stress_param[i], gaE})/d(trial_stress_param[j])
   */
  std::vector<std::vector<Real>> _dvar_dtrial;

  /**
   * The state (ok_sp, ok_intnl) is known to be admissible, so
   * ok_sp are stress_params that are "OK".  If the strain_increment
   * is applied in substeps then ok_sp is updated after each
   * sub strain_increment is applied and the return-map is successful.
   * At the end of the entire return-map process _ok_sp will contain
   * the stress_params where (_ok_sp, _intnl) is admissible.
   */
  std::vector<Real> _ok_sp;

  /**
   * The state (ok_sp, ok_intnl) is known to be admissible
   */
  std::vector<Real> _ok_intnl;

  /**
   * _del_stress_params = trial_stress_params - ok_sp
   * This is fixed at the beginning of the return-map process,
   * irrespective of substepping.  The return-map problem is:
   * apply del_stress_params to stress_prams, and then find
   * an admissible (returned) stress_params and gaE
   */
  std::vector<Real> _del_stress_params;

  /**
   * The current values of the stress params during the Newton-Raphson
   */
  std::vector<Real> _current_sp;

  /**
   * The current values of the internal params during the Newton-Raphson
   */
  std::vector<Real> _current_intnl;

private:
  /**
   * The type of smoother function
   */
  enum class SmootherFunctionType
  {
    cos,
    poly1,
    poly2,
    poly3
  } _smoother_function_type;
};
