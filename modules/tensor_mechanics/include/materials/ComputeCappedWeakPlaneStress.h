/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECAPPEDWEAKPLANESTRESS_H
#define COMPUTECAPPEDWEAKPLANESTRESS_H

#include "ComputeStressBase.h"
#include "TensorMechanicsHardeningModel.h"

#include <array>

class ComputeCappedWeakPlaneStress;

template<>
InputParameters validParams<ComputeCappedWeakPlaneStress>();

/**
 * ComputeCappedWeakPlaneStress performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped weak-plane plasticity
 *
 * It assumes various things about the elasticity tensor.
 * E(i,i,j,k) = 0 except if k=j
 * E(0,0,i,j) = E(1,1,i,j)
 */
class ComputeCappedWeakPlaneStress :
  public ComputeStressBase
{
public:
  ComputeCappedWeakPlaneStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress() override;
  virtual void initQpStatefulProperties() override;

  /// Number of variables = 2 = (p, q)
  constexpr static int _num_pq = 2;

  /// Number of yield functions
  constexpr static unsigned _num_yf = 3;

  /// Number of internal parameters
  constexpr static unsigned _num_intnl = 2;

  /// Number of equations in the Return-map Newton-Raphson
  constexpr static int _num_rhs = 3;

  /// Internal dimensionality of tensors (currently this is 3 throughout tensor_mechanics)
  constexpr static unsigned _tensor_dimensionality = 3;

  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _cohesion;

  /// Hardening model for tan(phi)
  const TensorMechanicsHardeningModel & _tan_phi;

  /// Hardening model for tan(psi)
  const TensorMechanicsHardeningModel & _tan_psi;

  /// Hardening model for tensile strength
  const TensorMechanicsHardeningModel & _tstrength;

  /// Hardening model for compressive strength
  const TensorMechanicsHardeningModel & _cstrength;

  /// The cone vertex is smoothed by this amount
  const Real _small_smoother2;

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
  enum class TangentOperatorEnum {
    elastic, nonlinear
  } _tangent_operator_type;

  /// Initialise the NR proceedure from a guess coming from perfect plasticity
  const bool _perfect_guess;

  /** This allows some simplification in the return-map process.
   * If the tensile yield function yf[1] >= 0 at the trial stress then
   * clearly the quadpoint is not going to fail in compression, so
   * _stress_return_type is set to no_compression, and every time
   * the compressive yield function is evaluated it is set -(very large).
   * If the compressive yield function yf[2] >= 0 at the trial stress
   * then clearly the quadpoint is not going to fail in tension, so
   * _stress_return_type is set to no_tension, and every time the
   * tensile yield function is evaluated it is set -(very large).
   * Otherwise (and at the very end after return-map) _stress_return_type
   * is set to nothing_special.
   */
  enum class StressReturnType {
    nothing_special, no_compression, no_tension
  } _stress_return_type;

  /** In order to help the Newton-Raphson procedure, the applied
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
  MaterialProperty<std::vector<Real> > & _intnl;

  /// old values of internal parameters
  MaterialProperty<std::vector<Real> > & _intnl_old;

  /// yield functions
  MaterialProperty<std::vector<Real> > & _yf;

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

  /** Computes the values of the yield functions, given stress and intnl
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] yf The yield function values (yf[0] = shear, yf[1] = tensile, yf[2] = compressive)
   */
  virtual void unsmoothedYieldFunctions(Real p, Real q, const std::vector<Real> & intnl, std::vector<Real> & yf) const;

  /** Computes the values of the yield functions, given stress and intnl, and puts them into _all_q
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   */
  virtual void unsmoothedYieldFunctions(Real p, Real q, const std::vector<Real> & intnl);

  /** Computes d(yieldFunctions)/d(p, q), and puts them into _all_q
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   */
  virtual void dyieldFunctions(Real p, Real q, const std::vector<Real> & intnl);

  /** Computes d(yieldFunctions)/d(intnl), and puts them into _all_q
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   */
  virtual void dyieldFunctions_di(Real p, Real q, const std::vector<Real> & intnl);

  /** Computes d(flowPotential)/d(p, q), and puts them into _all_q
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   */
  virtual void dflowPotential(Real p, Real q, const std::vector<Real> & intnl);

  /** Computes d2(flowPotential)/d(p, q)/d(p, q), and puts them into _all_q
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   */
  virtual void d2flowPotential(Real p, Real q, const std::vector<Real> & intnl);

  /** Computes d2(flowPotential)/d(p, q)/dintnl, and puts them into _all_q
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   */
  virtual void d2flowPotential_di(Real p, Real q, const std::vector<Real> & intnl);

  /** Computes the smoothed yield function
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @return The smoothed yield function value
   */
  virtual Real yieldF(Real p, Real q, const std::vector<Real> & intnl) const;

  /** Smooths yield functions
   */
  Real ismoother(Real f_diff) const;

  /** Derivative of ismoother
   */
  Real smoother(Real f_diff) const;

  /** Derivative of smoother
   */
  Real dsmoother(Real f_diff) const;

  /** Performs the return-map algorithm
   */
  virtual void returnMap();

  struct f_and_derivs
  {
    Real f;
    std::vector<Real> df;
    std::vector<Real> df_di;
    std::vector<Real> dg;
    std::vector<std::vector<Real> > d2g;
    std::vector<std::vector<Real> > d2g_di;

    f_and_derivs(): f_and_derivs(0, 0)
    {}

    f_and_derivs(unsigned num_var, unsigned num_intnl):
      f(0.0),
      df(num_var),
      df_di(num_intnl),
      dg(num_var),
      d2g(num_var),
      d2g_di(num_var)
    {
      for (unsigned i = 0; i < num_var; ++i)
      {
        d2g[i].assign(num_var, 0.0);
        d2g_di[i].assign(num_intnl, 0.0);
      }
    }

    bool operator < (const f_and_derivs & fd) const
    {
      return f < fd.f;
    }
  };

  /** Calculates _all_q, and then performs the smoothing scheme
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl Internal parameters
   * @return The smoothed yield function and derivatives
   */
  f_and_derivs smoothAllQuantities(Real p, Real q, const std::vector<Real> & intnl);

  /** Calculates _dp_dpt, _dp_dqt, etc for the (sub)strain increment
   * @param elastic_only whether this was an elastic step: if so then the updates to _dp_dpt, etc are fairly trivial
   * @param q the returned value of q for this (sub)strain increment
   * @param q_trial the trial value of q for this (sub)strain increment
   * @param gaE the value of gaE that came from this (sub)strain increment
   * @param smoothed_q contains the yield function and derivatives evaluated at (p, q)
   * @param step_size size of this (sub)strain increment
   */
  void dVardTrial(bool elastic_only, Real q, Real q_trial, Real gaE, const f_and_derivs & smoothed_q, Real step_size);

  /** Calculates the consistent tangent operator, _Jacobian_mult
   * @param q returned value of q after all Newton-Raphson has completed
   * @param q_trial the trial value of q for this strain increment
   * @param trial20 the trial value of stress_zx for this strain increment
   * @param trial21 the trial value of stress_zy for this strain increment
   * @param gaE the total value of that came from this strain increment
   * @param smoothed_q contains the yield function and derivatives evaluated at (p, q)
   */
  void consistentTangentOperator(Real q, Real q_trial, Real trial20, Real trial21, Real gaE, const f_and_derivs & smoothed_q);

  /** Performs a line-search to find (p, q)
   * Upon entry, this assumes that _rhs contains the Newton-Raphson full step (ie nrStep should have been called)
   * Upon exit, _rhs will contain the updated _rhs values ready for the next Newton-Raphson step,
   * Also _all_q and _intnl will be calculated at the new (p, q)
   * @param res2 the residual-squared, both as an input and output
   * @param gaE Upon input the value of gaE predicted from Newton Raphson.  Upon exit this will hold the value coming from the line-search
   * @param p Upon input the value of p predicted from Newton Raphson.  Upon exit this will be p coming from the line-search
   * @param q Upon input the value of q predicted from Newton Raphson.  Upon exit this will be q coming from the line-search
   * @param p_trial Trial value of p for this (sub)strain increment
   * @param q_trial Trial value of q for this (sub)strain increment
   * @param smoothed_q Upon input, the value of the smoothed yield function and derivatives at the prior-to-Newton configuration.  Upon exit this is evaluated at the new (p, q, intnl)
   * @param intnl_ok The value of _intnl from either the start of this (sub)strain increment
   */
  int lineSearch(Real & res2, Real & gaE, Real & p, Real & q, Real q_trial, Real p_trial, f_and_derivs & smoothed_q, const std::array<Real, 2> intnl_ok);

  /** Performs a Newton-Raphson step to zero _rhs
   * Upon return, _rhs will contain the solution.
   * @param smoothed_q The value of the smoothed yield function and derivatives prior to this Newton-Raphson step
   * @param q_trial The trial value of q for this (sub)strain increment
   * @param q The current value of q during the Newton-Raphson process
   * @param gaE The current value of ga during the Newton-Raphson process
   */
  int nrStep(const f_and_derivs & smoothed_q, Real q_trial, Real q, Real gaE);

  /** Calculates _rhs
   * 0 = _rhs[0] = f(p, q, intnl) = yield function
   * 0 = _rhs[1] = p - p_trial + Ezzzz * ga * dg/dp = normality in p direction
   * 0 = _rhs[2] = q - q_trial + Ezxzx * ga * dg/dq = normality in q direction
   * @param p_trial The trial value of p for this (sub)strain increment
   * @param q_trial The trial value of q for this (sub)strain increment
   * @param p The current value of p during the Newton-Raphson process
   * @param q The current value of q during the Newton-Raphson process
   * @param gaE The current value of ga during the Newton-Raphson process
   * @param smoothed_q Holds the current value of yield function and derivatives evaluated at (p, q, _intnl)
   */
  Real calculateRHS(Real p_trial, Real q_trial, Real p, Real q, Real gaE, const f_and_derivs & smoothed_q);

  /** Initialises (p, q, gaE) depending on _perfect_guess.
   * If _perfect_guess=false then p=p_trial, q=q_trial, gaE=0,
   * otherwise perfect plasticity (with no smoothing) is used to find a good initialisation
   * @param p_trial The trial value of p for this (sub)strain increment
   * @param q_trial The trial value of q for this (sub)strain increment
   * @param p The initialised value of p
   * @param q The initialised value of q
   * @param gaE The initialised value of gaE
   */
  void initialiseVars(Real p_trial, Real q_trial, Real & p, Real & q, Real & gaE);

  /**
   * Performs any necessary cleaning-up, then throw MooseException(message)
   * @param message The message to using in MooseException
   */
  virtual void errorHandler(const std::string & message);

private:
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
};

#endif //COMPUTECAPPEDWEAKPLANESTRESS_H
