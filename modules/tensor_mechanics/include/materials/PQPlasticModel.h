/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PQPLASTICMODEL_H
#define PQPLASTICMODEL_H

#include "ComputeStressBase.h"

#include <array>

class PQPlasticModel;

template <>
InputParameters validParams<PQPlasticModel>();

/**
 * PQPlasticModel performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe (p, q) plasticity.  That is,
 * for plastic models where the yield function and flow
 * directions only depend on two parameters, p and q, that
 * are themselves functions of stress
 */
class PQPlasticModel : public ComputeStressBase
{
public:
  PQPlasticModel(const InputParameters & parameters, unsigned num_yf, unsigned num_intnl);

protected:
  virtual void computeQpStress() override;
  virtual void initQpStatefulProperties() override;

  /// Number of variables = 2 = (p, q)
  constexpr static int _num_pq = 2;

  /// Number of equations in the Return-map Newton-Raphson
  constexpr static int _num_rhs = 3;

  /// Internal dimensionality of tensors (currently this is 3 throughout tensor_mechanics)
  constexpr static unsigned _tensor_dimensionality = 3;

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

  /// The yield-function tolerance
  const Real _f_tol;

  /// Square of the yield-function tolerance
  const Real _f_tol2;

  /// The type of tangent operator to return.  tangent operator = d(stress_rate)/d(strain_rate).
  enum class TangentOperatorEnum
  {
    elastic,
    nonlinear
  } _tangent_operator_type;

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
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// internal parameters.  intnl[0] = shear.  intnl[1] = tensile.
  MaterialProperty<std::vector<Real>> & _intnl;

  /// old values of internal parameters
  MaterialProperty<std::vector<Real>> & _intnl_old;

  /// yield functions
  MaterialProperty<std::vector<Real>> & _yf;

  /// Number of Newton-Raphson iterations used in the return-map
  MaterialProperty<Real> & _iter;

  /// Whether a line-search was needed in the latest Newton-Raphson process (1 if true, 0 otherwise)
  MaterialProperty<Real> & _linesearch_needed;

  /// strain increment (coming from ComputeIncrementalSmallStrain, for example)
  const MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Rotation increment (coming from ComputeIncrementalSmallStrain, for example)
  const MaterialProperty<RankTwoTensor> & _rotation_increment;

  /// Old value of stress
  MaterialProperty<RankTwoTensor> & _stress_old;

  /// Old value of elastic strain
  MaterialProperty<RankTwoTensor> & _elastic_strain_old;

  /// Trial value of p
  Real _p_trial;

  /// Trial value of q
  Real _q_trial;

  /// State at (p_ok, q_ok, _intnl_ok) is known to be admissible
  std::vector<Real> _intnl_ok;

  /// _dintnl[i][j] = d(intnl[i])/d(variable j), where variable0=p and variable1=q
  std::vector<std::vector<Real>> _dintnl;

  /// elasticity tensor in p direction
  Real _Epp;

  /// elasticity tensor in q direction
  Real _Eqq;

  struct f_and_derivs
  {
    Real f;
    std::vector<Real> df;
    std::vector<Real> df_di;
    std::vector<Real> dg;
    std::vector<std::vector<Real>> d2g;
    std::vector<std::vector<Real>> d2g_di;

    f_and_derivs() : f_and_derivs(0, 0) {}

    f_and_derivs(unsigned num_var, unsigned num_intnl)
      : f(0.0), df(num_var), df_di(num_intnl), dg(num_var), d2g(num_var), d2g_di(num_var)
    {
      for (unsigned i = 0; i < num_var; ++i)
      {
        d2g[i].assign(num_var, 0.0);
        d2g_di[i].assign(num_intnl, 0.0);
      }
    }

    bool operator<(const f_and_derivs & fd) const { return f < fd.f; }
  };

  /**
   * Computes the smoothed yield function
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @return The smoothed yield function value
   */
  Real yieldF(Real p, Real q, const std::vector<Real> & intnl) const;

  /**
   * Smooths yield functions
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
   * Performs the return-map algorithm
   */
  void returnMap();

  /**
   * Calculates _all_q, and then performs the smoothing scheme
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl Internal parameters
   * @return The smoothed yield function and derivatives
   */
  f_and_derivs smoothAllQuantities(Real p, Real q, const std::vector<Real> & intnl);

  /**
   * Calculates _dp_dpt, _dp_dqt, etc for the (sub)strain increment
   * @param elastic_only whether this was an elastic step: if so then the updates to _dp_dpt, etc
   * are fairly trivial
   * @param p_trial the trial value of p for this (sub)strain increment
   * @param q_trial the trial value of q for this (sub)strain increment
   * @param p the returned value of p for this (sub)strain increment
   * @param q the returned value of q for this (sub)strain increment
   * @param gaE the value of gaE that came from this (sub)strain increment
   * @param intnl the value of the internal parameters at the returned position
   * @param smoothed_q contains the yield function and derivatives evaluated at (p, q)
   * @param step_size size of this (sub)strain increment
   */
  void dVardTrial(bool elastic_only,
                  Real p_trial,
                  Real q_trial,
                  Real p,
                  Real q,
                  Real gaE,
                  const std::vector<Real> & intnl,
                  const f_and_derivs & smoothed_q,
                  Real step_size);

  /**
   * Performs a line-search to find (p, q)
   * Upon entry, this assumes that _rhs contains the Newton-Raphson full step (ie nrStep should have
   * been called)
   * Upon exit, _rhs will contain the updated _rhs values ready for the next Newton-Raphson step,
   * Also _all_q and _intnl will be calculated at the new (p, q)
   * @param res2 the residual-squared, both as an input and output
   * @param gaE Upon input the value of gaE predicted from Newton Raphson.  Upon exit this will hold
   * the value coming from the line-search
   * @param p Upon input the value of p predicted from Newton Raphson.  Upon exit this will be p
   * coming from the line-search
   * @param q Upon input the value of q predicted from Newton Raphson.  Upon exit this will be q
   * coming from the line-search
   * @param p_trial Trial value of p for this (sub)strain increment
   * @param q_trial Trial value of q for this (sub)strain increment
   * @param smoothed_q Upon input, the value of the smoothed yield function and derivatives at the
   * prior-to-Newton configuration.  Upon exit this is evaluated at the new (p, q, intnl)
   * @param intnl_ok The value of _intnl from either the start of this (sub)strain increment
   */
  int lineSearch(Real & res2,
                 Real & gaE,
                 Real & p,
                 Real & q,
                 Real p_trial,
                 Real q_trial,
                 f_and_derivs & smoothed_q,
                 const std::vector<Real> & intnl_ok);

  /**
   * Performs a Newton-Raphson step to attempt to zero _rhs
   * Upon return, _rhs will contain the solution.
   * @param smoothed_q The value of the smoothed yield function and derivatives prior to this
   * Newton-Raphson step
   * @param p_trial The trial value of p for this (sub)strain increment
   * @param q_trial The trial value of q for this (sub)strain increment
   * @param p The current value of p during the Newton-Raphson process
   * @param q The current value of q during the Newton-Raphson process
   * @param gaE The current value of ga during the Newton-Raphson process
   */
  int nrStep(const f_and_derivs & smoothed_q, Real p_trial, Real q_trial, Real p, Real q, Real gaE);

  /**
   * Calculates _rhs
   * 0 = _rhs[0] = f(p, q, intnl) = yield function
   * 0 = _rhs[1] = p - p_trial + Epp * ga * dg/dp = normality in p direction
   * 0 = _rhs[2] = q - q_trial + Eqq * ga * dg/dq = normality in q direction
   * @param p_trial The trial value of p for this (sub)strain increment
   * @param q_trial The trial value of q for this (sub)strain increment
   * @param p The current value of p during the Newton-Raphson process
   * @param q The current value of q during the Newton-Raphson process
   * @param gaE The current value of ga during the Newton-Raphson process
   * @param smoothed_q Holds the current value of yield function and derivatives evaluated at (p, q,
   * _intnl)
   */
  Real calculateRHS(
      Real p_trial, Real q_trial, Real p, Real q, Real gaE, const f_and_derivs & smoothed_q);

  /**
   * Derivative of -RHS with respect to the variables, placed
   * into an array ready for solving the linear system using
   * LAPACK gsev
   * @param smoothed_q Holds the current value of yield function and derivatives evaluated at (p, q,
   * _intnl)
   * @param dintnl The derivatives of the internal parameters wrt p and q
   * @param gaE The current value of ga during the Newton-Raphson process
   * @param jac The outputted derivatives
   */
  void dnRHSdVar(const f_and_derivs & smoothed_q,
                 const std::vector<std::vector<Real>> & dintnl,
                 Real gaE,
                 std::array<double, _num_rhs * _num_rhs> & jac) const;

  /**
   * Performs any necessary cleaning-up, then throw MooseException(message)
   * @param message The message to using in MooseException
   */
  virtual void errorHandler(const std::string & message);

  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dgaE_dpt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dp_dpt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dq_dpt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dgaE_dqt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dp_dqt;
  /// derivative of Variable with respect to trial variable (used in consistent-tangent-operator calculation)
  Real _dq_dqt;

  /** A Newton-Raphson-with-linesearch is used for the return-map
   * process.  Three equations must be solved:
   * 0 = _rhs[0] = f(p, q, intnl) = yield function
   * 0 = _rhs[1] = p - p_trial + Ezzzz * ga * dg/dp = normality in p direction
   * 0 = _rhs[2] = q - q_trial + Ezxzx * ga * dg/dq = normality in q direction
   * Not only does _rhs hold the current value of the right-hand-side
   * in these equations, but the PETSc-LAPACK gesv routine puts the
   * changes of the variables into the _rhs too
   */
  std::array<Real, _num_rhs> _rhs;

  /** Holds the yield function, derivatives and flow information for the
   * three yield functions of this model
   */
  std::vector<f_and_derivs> _all_q;

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
                           std::vector<f_and_derivs> & all_q) const = 0;

  /**
   * Derived classes may employ this function to record stuff or do
   * other computations prior to the return-mapping algorithm.  We
   * know that (p_trial, q_trial, intnl_old) is inadmissible.
   * @param p_trial Trial value of p
   * @param q_trial Trial value of q
   * @param stress_trial Trial stress tensor
   * @param intnl_old Old value of the internal parameters.
   * @param yf The yield functions at (p_trial, q_trial, intnl_old)
   */
  virtual void preReturnMap(Real p_trial,
                            Real q_trial,
                            const RankTwoTensor & stress_trial,
                            const std::vector<Real> & intnl_old,
                            const std::vector<Real> & yf);

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
  virtual void initialiseVars(Real p_trial,
                              Real q_trial,
                              const std::vector<Real> & intnl_old,
                              Real & p,
                              Real & q,
                              Real & gaE,
                              std::vector<Real> & intnl) const;

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

  /**
   * Computes p and q, given stress.  Derived classes must
   * override this
   * @param stress Stress tensor
   * @param p p stress
   * @param q q q stress
   */
  virtual void computePQ(const RankTwoTensor & stress, Real & p, Real & q) const = 0;

  /**
   * Set Epp and Eqq based on the elasticity tensor
   * Derived classes must override this
   * @param Eijkl elasticity tensor
   * @param[out] Epp Epp value
   * @param[out] Eqq Eqq value
   */
  virtual void setEppEqq(const RankFourTensor & Eijkl, Real & Epp, Real & Eqq) const = 0;

  /**
   * Derived classes may use this to perform calculations before
   * any return-map process is performed, for instance, to initialise
   * variables.
   * This is called at the very start of computeQpStress, even before
   * any checking for admissible stresses, etc, is performed
   */
  virtual void initialiseReturnProcess();

  /**
   * Derived classes may use this to perform calculations after the
   * return-map process has completed successfully in (p, q) space
   * but before the returned stress tensor has been performed.
   */
  virtual void finaliseReturnProcess();

  /**
   * Sets stress from the admissible parameters.
   * This is called after the return-map process has completed
   * successfully in (p, q) space, just after finaliseReturnProcess
   * has been called.
   * Derived classes may override this function
   * @param stress_trial The trial value of stress
   * @param p_ok Returned value of p
   * @param q_ok Returned value of q
   * @param gaE Value of gaE induced by the return (gaE = gamma * Epp)
   * @param smoothed_q Holds the current value of yield function and derivatives evaluated at (p_ok,
   * q_ok, _intnl)
   * @param stress[out] The returned value of the stress tensor
   */
  virtual void setStressAfterReturn(const RankTwoTensor & stress_trial,
                                    Real p_ok,
                                    Real q_ok,
                                    Real gaE,
                                    const std::vector<Real> & intnl,
                                    const f_and_derivs & smoothed_q,
                                    RankTwoTensor & stress) const;

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
   * @param[out] cto The consistent tangent operator
   */
  virtual void consistentTangentOperator(const RankTwoTensor & stress_trial,
                                         Real p_trial,
                                         Real q_trial,
                                         const RankTwoTensor & stress,
                                         Real p,
                                         Real q,
                                         Real gaE,
                                         const f_and_derivs & smoothed_q,
                                         RankFourTensor & cto) const;

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

#endif // PQPLASTICMODEL_H
