//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiParameterPlasticityStressUpdate.h"
#include "Conversion.h" // for stringify
#include "MooseEnum.h"  // for enum

// libMesh includes
#include "libmesh/utility.h" // for Utility::pow

// PETSc includes
#include <petscblaslapack.h> // LAPACKgesv_

InputParameters
MultiParameterPlasticityStressUpdate::validParams()
{
  InputParameters params = StressUpdateBase::validParams();
  params.addClassDescription("Return-map and Jacobian algorithms for plastic models where the "
                             "yield function and flow directions depend on multiple functions of "
                             "stress");
  params.addRangeCheckedParam<unsigned int>(
      "max_NR_iterations",
      20,
      "max_NR_iterations>0",
      "Maximum number of Newton-Raphson iterations allowed during the return-map algorithm");
  params.addParam<bool>("perform_finite_strain_rotations",
                        false,
                        "Tensors are correctly rotated "
                        "in finite-strain simulations.  "
                        "For optimal performance you can "
                        "set this to 'false' if you are "
                        "only ever using small strains");
  params.addRequiredParam<Real>(
      "smoothing_tol",
      "Intersections of the yield surfaces will be smoothed by this amount (this "
      "is measured in units of stress).  Often this is related to other physical "
      "parameters (eg, 0.1*cohesion) but it is important to set this small enough "
      "so that the individual yield surfaces do not mix together in the smoothing "
      "process to produce a result where no stress is admissible (for example, "
      "mixing together tensile and compressive failure envelopes).");
  params.addRequiredParam<Real>("yield_function_tol",
                                "The return-map process will be deemed to have converged if all "
                                "yield functions are within yield_function_tol of zero.  If this "
                                "is set very low then precision-loss might be encountered: if the "
                                "code detects precision loss then it also deems the return-map "
                                "process has converged.");
  MooseEnum tangent_operator("elastic nonlinear", "nonlinear");
  params.addParam<Real>("min_step_size",
                        1.0,
                        "In order to help the Newton-Raphson procedure, the applied strain "
                        "increment may be applied in sub-increments of size greater than this "
                        "value.  Usually it is better for Moose's nonlinear convergence to "
                        "increase max_NR_iterations rather than decrease this parameter.");
  params.addParam<bool>("warn_about_precision_loss",
                        false,
                        "Output a message to the console every "
                        "time precision-loss is encountered "
                        "during the Newton-Raphson process");
  params.addParam<std::vector<Real>>("admissible_stress",
                                     "A single admissible value of the value of the stress "
                                     "parameters for internal parameters = 0.  This is used "
                                     "to initialize the return-mapping algorithm during the first "
                                     "nonlinear iteration.  If not given then it is assumed that "
                                     "stress parameters = 0 is admissible.");
  MooseEnum smoother_fcn_enum("cos poly1 poly2 poly3", "cos");
  params.addParam<MooseEnum>("smoother_function_type",
                             smoother_fcn_enum,
                             "Type of smoother function to use.  'cos' means (-a/pi)cos(pi x/2/a), "
                             "'polyN' means a polynomial of degree 2N+2");
  params.addParamNamesToGroup("smoother_function_type", "Advanced");
  return params;
}

MultiParameterPlasticityStressUpdate::MultiParameterPlasticityStressUpdate(
    const InputParameters & parameters, unsigned num_sp, unsigned num_yf, unsigned num_intnl)
  : StressUpdateBase(parameters),
    _num_sp(num_sp),
    _definitely_ok_sp(isParamValid("admissible_stress")
                          ? getParam<std::vector<Real>>("admissible_stress")
                          : std::vector<Real>(_num_sp, 0.0)),
    _Eij(num_sp, std::vector<Real>(num_sp)),
    _En(1.0),
    _Cij(num_sp, std::vector<Real>(num_sp)),
    _num_yf(num_yf),
    _num_intnl(num_intnl),
    _max_nr_its(getParam<unsigned>("max_NR_iterations")),
    _perform_finite_strain_rotations(getParam<bool>("perform_finite_strain_rotations")),
    _smoothing_tol(getParam<Real>("smoothing_tol")),
    _smoothing_tol2(Utility::pow<2>(getParam<Real>("smoothing_tol"))),
    _f_tol(getParam<Real>("yield_function_tol")),
    _f_tol2(Utility::pow<2>(getParam<Real>("yield_function_tol"))),
    _min_step_size(getParam<Real>("min_step_size")),
    _step_one(declareRestartableData<bool>("step_one", true)),
    _warn_about_precision_loss(getParam<bool>("warn_about_precision_loss")),

    _plastic_strain(declareProperty<RankTwoTensor>(_base_name + "plastic_strain")),
    _plastic_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "plastic_strain")),
    _intnl(declareProperty<std::vector<Real>>(_base_name + "plastic_internal_parameter")),
    _intnl_old(
        getMaterialPropertyOld<std::vector<Real>>(_base_name + "plastic_internal_parameter")),
    _yf(declareProperty<std::vector<Real>>(_base_name + "plastic_yield_function")),
    _iter(declareProperty<Real>(_base_name +
                                "plastic_NR_iterations")), // this is really an unsigned int, but
                                                           // for visualisation i convert it to Real
    _max_iter_used(declareProperty<Real>(
        _base_name + "max_plastic_NR_iterations")), // this is really an unsigned int, but
                                                    // for visualisation i convert it to Real
    _max_iter_used_old(getMaterialPropertyOld<Real>(_base_name + "max_plastic_NR_iterations")),
    _linesearch_needed(
        declareProperty<Real>(_base_name + "plastic_linesearch_needed")), // this is really a
                                                                          // boolean, but for
                                                                          // visualisation i
                                                                          // convert it to Real
    _trial_sp(num_sp),
    _stress_trial(RankTwoTensor()),
    _rhs(num_sp + 1),
    _dvar_dtrial(num_sp + 1, std::vector<Real>(num_sp, 0.0)),
    _ok_sp(num_sp),
    _ok_intnl(num_intnl),
    _del_stress_params(num_sp),
    _current_sp(num_sp),
    _current_intnl(num_intnl),
    _smoother_function_type(
        parameters.get<MooseEnum>("smoother_function_type").getEnum<SmootherFunctionType>())
{
  if (_definitely_ok_sp.size() != _num_sp)
    mooseError("MultiParameterPlasticityStressUpdate: admissible_stress parameter must consist of ",
               _num_sp,
               " numbers");
}

void
MultiParameterPlasticityStressUpdate::initQpStatefulProperties()
{
  _plastic_strain[_qp].zero();
  _intnl[_qp].assign(_num_intnl, 0);
  _yf[_qp].assign(_num_yf, 0);
  _iter[_qp] = 0.0;
  _max_iter_used[_qp] = 0.0;
  _linesearch_needed[_qp] = 0.0;
}

void
MultiParameterPlasticityStressUpdate::propagateQpStatefulProperties()
{
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
  std::copy(_intnl_old[_qp].begin(), _intnl_old[_qp].end(), _intnl[_qp].begin());
  _max_iter_used[_qp] = _max_iter_used_old[_qp];
}

void
MultiParameterPlasticityStressUpdate::updateState(RankTwoTensor & strain_increment,
                                                  RankTwoTensor & inelastic_strain_increment,
                                                  const RankTwoTensor & rotation_increment,
                                                  RankTwoTensor & stress_new,
                                                  const RankTwoTensor & stress_old,
                                                  const RankFourTensor & elasticity_tensor,
                                                  const RankTwoTensor & /*elastic_strain_old*/,
                                                  bool compute_full_tangent_operator,
                                                  RankFourTensor & tangent_operator)
{
  // Size _yf[_qp] appropriately
  _yf[_qp].assign(_num_yf, 0);
  // _plastic_strain and _intnl are usually sized appropriately because they are stateful, but this
  // Material may be used from a DiracKernel where stateful materials are not allowed.  The best we
  // can do is:
  if (_intnl[_qp].size() != _num_intnl)
    initQpStatefulProperties();

  initializeReturnProcess();

  if (_t_step >= 2)
    _step_one = false;

  // initially assume an elastic deformation
  std::copy(_intnl_old[_qp].begin(), _intnl_old[_qp].end(), _intnl[_qp].begin());

  _iter[_qp] = 0.0;
  _max_iter_used[_qp] = std::max(_max_iter_used[_qp], _max_iter_used_old[_qp]);
  _linesearch_needed[_qp] = 0.0;

  computeStressParams(stress_new, _trial_sp);
  yieldFunctionValuesV(_trial_sp, _intnl[_qp], _yf[_qp]);

  if (yieldF(_yf[_qp]) <= _f_tol)
  {
    _plastic_strain[_qp] = _plastic_strain_old[_qp];
    inelastic_strain_increment.zero();
    if (_fe_problem.currentlyComputingJacobian())
      tangent_operator = elasticity_tensor;
    return;
  }

  _stress_trial = stress_new;
  /* The trial stress must be inadmissible
   * so we need to return to the yield surface.  The following
   * equations must be satisfied.
   *
   * 0 = rhs[0] = S[0] - S[0]^trial + ga * E[0, i] * dg/dS[i]
   * 0 = rhs[1] = S[1] - S[1]^trial + ga * E[1, i] * dg/dS[i]
   * ...
   * 0 = rhs[N-1] = S[N-1] - S[N-1]^trial + ga * E[N-1, i] * dg/dS[i]
   * 0 = rhs[N] = f(S, intnl)
   *
   * as well as equations defining intnl parameters as functions of
   * stress_params, trial_stress_params and intnl_old
   *
   * The unknowns are S[0], ..., S[N-1], gaE, and the intnl parameters.
   * Here gaE = ga * _En (the _En serves to make gaE similar magnitude to S)
   * I find it convenient to solve the first N+1 equations for p, q and gaE,
   * while substituting the "intnl parameters" equations into these during the solve process
   */

  for (auto & deriv : _dvar_dtrial)
    deriv.assign(_num_sp, 0.0);

  preReturnMapV(_trial_sp, stress_new, _intnl_old[_qp], _yf[_qp], elasticity_tensor);

  setEffectiveElasticity(elasticity_tensor);

  if (_step_one)
    std::copy(_definitely_ok_sp.begin(), _definitely_ok_sp.end(), _ok_sp.begin());
  else
    computeStressParams(stress_old, _ok_sp);
  std::copy(_intnl_old[_qp].begin(), _intnl_old[_qp].end(), _ok_intnl.begin());

  // Return-map problem: must apply the following changes in stress_params,
  // and find the returned stress_params and gaE
  for (unsigned i = 0; i < _num_sp; ++i)
    _del_stress_params[i] = _trial_sp[i] - _ok_sp[i];

  Real step_taken = 0.0; // amount of del_stress_params that we've applied and the return-map
                         // problem has succeeded
  Real step_size = 1.0;  // potentially can apply del_stress_params in substeps
  Real gaE_total = 0.0;

  // current values of the yield function, derivatives, etc
  yieldAndFlow smoothed_q;

  // In the following sub-stepping procedure it is possible that
  // the last step is an elastic step, and therefore smoothed_q won't
  // be computed on the last step, so we have to compute it.
  bool smoothed_q_calculated = false;

  while (step_taken < 1.0 && step_size >= _min_step_size)
  {
    if (1.0 - step_taken < step_size)
      // prevent over-shoots of substepping
      step_size = 1.0 - step_taken;

    // trial variables in terms of admissible variables
    for (unsigned i = 0; i < _num_sp; ++i)
      _trial_sp[i] = _ok_sp[i] + step_size * _del_stress_params[i];

    // initialize variables that are to be found via Newton-Raphson
    _current_sp = _trial_sp;
    Real gaE = 0.0;

    // flags indicating failure of Newton-Raphson and line-search
    int nr_failure = 0;
    int ls_failure = 0;

    // NR iterations taken in this substep
    unsigned step_iter = 0;

    // The residual-squared for the line-search
    Real res2 = 0.0;

    if (step_size < 1.0 && yieldF(_trial_sp, _ok_intnl) <= _f_tol)
      // This is an elastic step
      // The "step_size < 1.0" in above condition is for efficiency: we definitely
      // know that this is a plastic step if step_size = 1.0
      smoothed_q_calculated = false;
    else
    {
      // this is a plastic step

      // initialize current_sp, gaE and current_intnl based on the non-smoothed situation
      initializeVarsV(_trial_sp, _ok_intnl, _current_sp, gaE, _current_intnl);
      // and find the smoothed yield function, flow potential and derivatives
      smoothed_q = smoothAllQuantities(_current_sp, _current_intnl);
      smoothed_q_calculated = true;
      calculateRHS(_trial_sp, _current_sp, gaE, smoothed_q, _rhs);
      res2 = calculateRes2(_rhs);

      // Perform a Newton-Raphson with linesearch to get current_sp, gaE, and also smoothed_q
      while (res2 > _f_tol2 && step_iter < _max_nr_its && nr_failure == 0 && ls_failure == 0)
      {
        // solve the linear system and store the answer (the "updates") in rhs
        nr_failure = nrStep(smoothed_q, _trial_sp, _current_sp, _current_intnl, gaE, _rhs);
        if (nr_failure != 0)
          break;

        // handle precision loss
        if (precisionLoss(_rhs, _current_sp, gaE))
        {
          if (_warn_about_precision_loss)
          {
            Moose::err << "MultiParameterPlasticityStressUpdate: precision-loss in element "
                       << _current_elem->id() << " quadpoint=" << _qp << ".  Stress_params =";
            for (unsigned i = 0; i < _num_sp; ++i)
              Moose::err << " " << _current_sp[i];
            Moose::err << " gaE = " << gaE << "\n";
          }
          res2 = 0.0;
          break;
        }

        // apply (parts of) the updates, re-calculate smoothed_q, and res2
        ls_failure = lineSearch(res2,
                                _current_sp,
                                gaE,
                                _trial_sp,
                                smoothed_q,
                                _ok_intnl,
                                _current_intnl,
                                _rhs,
                                _linesearch_needed[_qp]);
        step_iter++;
      }
    }
    if (res2 <= _f_tol2 && step_iter < _max_nr_its && nr_failure == 0 && ls_failure == 0 &&
        gaE >= 0.0)
    {
      // this Newton-Raphson worked fine, or this was an elastic step
      std::copy(_current_sp.begin(), _current_sp.end(), _ok_sp.begin());
      gaE_total += gaE;
      step_taken += step_size;
      setIntnlValuesV(_trial_sp, _ok_sp, _ok_intnl, _intnl[_qp]);
      std::copy(_intnl[_qp].begin(), _intnl[_qp].end(), _ok_intnl.begin());
      // calculate dp/dp_trial, dp/dq_trial, etc, for Jacobian
      dVardTrial(!smoothed_q_calculated,
                 _trial_sp,
                 _ok_sp,
                 gaE,
                 _ok_intnl,
                 smoothed_q,
                 step_size,
                 compute_full_tangent_operator,
                 _dvar_dtrial);
      if (static_cast<Real>(step_iter) > _iter[_qp])
        _iter[_qp] = static_cast<Real>(step_iter);
      if (static_cast<Real>(step_iter) > _max_iter_used[_qp])
        _max_iter_used[_qp] = static_cast<Real>(step_iter);
      step_size *= 1.1;
    }
    else
    {
      // Newton-Raphson + line-search process failed
      step_size *= 0.5;
    }
  }

  if (step_size < _min_step_size)
    errorHandler("MultiParameterPlasticityStressUpdate: Minimum step-size violated");

  // success!
  finalizeReturnProcess(rotation_increment);
  yieldFunctionValuesV(_ok_sp, _intnl[_qp], _yf[_qp]);

  if (!smoothed_q_calculated)
    smoothed_q = smoothAllQuantities(_ok_sp, _intnl[_qp]);

  setStressAfterReturnV(
      _stress_trial, _ok_sp, gaE_total, _intnl[_qp], smoothed_q, elasticity_tensor, stress_new);

  setInelasticStrainIncrementAfterReturn(_stress_trial,
                                         gaE_total,
                                         smoothed_q,
                                         elasticity_tensor,
                                         stress_new,
                                         inelastic_strain_increment);

  strain_increment = strain_increment - inelastic_strain_increment;
  _plastic_strain[_qp] = _plastic_strain_old[_qp] + inelastic_strain_increment;

  if (_fe_problem.currentlyComputingJacobian())
    // for efficiency, do not compute the tangent operator if not currently computing Jacobian
    consistentTangentOperatorV(_stress_trial,
                               _trial_sp,
                               stress_new,
                               _ok_sp,
                               gaE_total,
                               smoothed_q,
                               elasticity_tensor,
                               compute_full_tangent_operator,
                               _dvar_dtrial,
                               tangent_operator);
}

MultiParameterPlasticityStressUpdate::yieldAndFlow
MultiParameterPlasticityStressUpdate::smoothAllQuantities(const std::vector<Real> & stress_params,
                                                          const std::vector<Real> & intnl) const
{
  std::vector<yieldAndFlow> all_q(_num_yf, yieldAndFlow(_num_sp, _num_intnl));
  computeAllQV(stress_params, intnl, all_q);

  /* This routine holds the key to my smoothing strategy.  It
   * may be proved that this smoothing strategy produces a
   * yield surface that is both C2 differentiable and convex,
   * assuming the individual yield functions are C2 and
   * convex too.
   * Of course all the derivatives must also be smoothed.
   * Also, I assume that d(flow potential)/dstress gets smoothed
   * by the Yield Function (which produces a C2 flow potential).
   * See the line identified in the loop below.
   * Only time will tell whether this is a good strategy, but it
   * works well in all tests so far.  Convexity is irrelevant
   * for the non-associated case, but at least the return-map
   * problem should always have a unique solution.
   * For two yield functions+flows, labelled 1 and 2, we
   * should have
   * d(g1 - g2) . d(f1 - f2) >= 0
   * If not then the return-map problem for even the
   * multi-surface plasticity with no smoothing won't have a
   * unique solution.  If the multi-surface plasticity has
   * a unique solution then the smoothed version defined
   * below will too.
   */

  // res_f is the index that contains the smoothed yieldAndFlow
  std::size_t res_f = 0;

  for (std::size_t a = 1; a < all_q.size(); ++a)
  {
    if (all_q[res_f].f >= all_q[a].f + _smoothing_tol)
      // no smoothing is needed: res_f is already indexes the largest yield function
      continue;
    else if (all_q[a].f >= all_q[res_f].f + _smoothing_tol)
    {
      // no smoothing is needed, and res_f needs to index to all_q[a]
      res_f = a;
      continue;
    }
    else
    {
      // smoothing is required
      const Real f_diff = all_q[res_f].f - all_q[a].f;
      const Real ism = ismoother(f_diff);
      const Real sm = smoother(f_diff);
      const Real dsm = dsmoother(f_diff);
      // we want: all_q[res_f].f = 0.5 * all_q[res_f].f + all_q[a].f + _smoothing_tol) + ism,
      // but we have to do the derivatives first
      for (unsigned i = 0; i < _num_sp; ++i)
      {
        for (unsigned j = 0; j < _num_sp; ++j)
          all_q[res_f].d2g[i][j] =
              0.5 * (all_q[res_f].d2g[i][j] + all_q[a].d2g[i][j]) +
              dsm * (all_q[res_f].df[j] - all_q[a].df[j]) * (all_q[res_f].dg[i] - all_q[a].dg[i]) +
              sm * (all_q[res_f].d2g[i][j] - all_q[a].d2g[i][j]);
        for (unsigned j = 0; j < _num_intnl; ++j)
          all_q[res_f].d2g_di[i][j] = 0.5 * (all_q[res_f].d2g_di[i][j] + all_q[a].d2g_di[i][j]) +
                                      dsm * (all_q[res_f].df_di[j] - all_q[a].df_di[j]) *
                                          (all_q[res_f].dg[i] - all_q[a].dg[i]) +
                                      sm * (all_q[res_f].d2g_di[i][j] - all_q[a].d2g_di[i][j]);
      }
      for (unsigned i = 0; i < _num_sp; ++i)
      {
        all_q[res_f].df[i] = 0.5 * (all_q[res_f].df[i] + all_q[a].df[i]) +
                             sm * (all_q[res_f].df[i] - all_q[a].df[i]);
        // whether the following (smoothing g with f's smoother) is a good strategy remains to be
        // seen...
        all_q[res_f].dg[i] = 0.5 * (all_q[res_f].dg[i] + all_q[a].dg[i]) +
                             sm * (all_q[res_f].dg[i] - all_q[a].dg[i]);
      }
      for (unsigned i = 0; i < _num_intnl; ++i)
        all_q[res_f].df_di[i] = 0.5 * (all_q[res_f].df_di[i] + all_q[a].df_di[i]) +
                                sm * (all_q[res_f].df_di[i] - all_q[a].df_di[i]);
      all_q[res_f].f = 0.5 * (all_q[res_f].f + all_q[a].f + _smoothing_tol) + ism;
    }
  }
  return all_q[res_f];
}

Real
MultiParameterPlasticityStressUpdate::ismoother(Real f_diff) const
{
  if (std::abs(f_diff) >= _smoothing_tol)
    return 0.0;
  switch (_smoother_function_type)
  {
    case SmootherFunctionType::cos:
      return -_smoothing_tol / M_PI * std::cos(0.5 * M_PI * f_diff / _smoothing_tol);
    case SmootherFunctionType::poly1:
      return 0.75 / _smoothing_tol *
             (0.5 * (Utility::pow<2>(f_diff) - _smoothing_tol2) -
              (_smoothing_tol2 / 12.0) * (Utility::pow<4>(f_diff / _smoothing_tol) - 1.0));
    case SmootherFunctionType::poly2:
      return 0.625 / _smoothing_tol *
             (0.5 * (Utility::pow<2>(f_diff) - _smoothing_tol2) -
              (_smoothing_tol2 / 30.0) * (Utility::pow<6>(f_diff / _smoothing_tol) - 1.0));
    case SmootherFunctionType::poly3:
      return (7.0 / 12.0 / _smoothing_tol) *
             (0.5 * (Utility::pow<2>(f_diff) - _smoothing_tol2) -
              (_smoothing_tol2 / 56.0) * (Utility::pow<8>(f_diff / _smoothing_tol) - 1.0));
    default:
      return 0.0;
  }
}

Real
MultiParameterPlasticityStressUpdate::smoother(Real f_diff) const
{
  if (std::abs(f_diff) >= _smoothing_tol)
    return 0.0;
  switch (_smoother_function_type)
  {
    case SmootherFunctionType::cos:
      return 0.5 * std::sin(f_diff * M_PI * 0.5 / _smoothing_tol);
    case SmootherFunctionType::poly1:
      return 0.75 / _smoothing_tol *
             (f_diff - (_smoothing_tol / 3.0) * Utility::pow<3>(f_diff / _smoothing_tol));
    case SmootherFunctionType::poly2:
      return 0.625 / _smoothing_tol *
             (f_diff - (_smoothing_tol / 5.0) * Utility::pow<5>(f_diff / _smoothing_tol));
    case SmootherFunctionType::poly3:
      return (7.0 / 12.0 / _smoothing_tol) *
             (f_diff - (_smoothing_tol / 7.0) * Utility::pow<7>(f_diff / _smoothing_tol));
    default:
      return 0.0;
  }
}

Real
MultiParameterPlasticityStressUpdate::dsmoother(Real f_diff) const
{
  if (std::abs(f_diff) >= _smoothing_tol)
    return 0.0;
  switch (_smoother_function_type)
  {
    case SmootherFunctionType::cos:
      return 0.25 * M_PI / _smoothing_tol * std::cos(f_diff * M_PI * 0.5 / _smoothing_tol);
    case SmootherFunctionType::poly1:
      return 0.75 / _smoothing_tol * (1.0 - Utility::pow<2>(f_diff / _smoothing_tol));
    case SmootherFunctionType::poly2:
      return 0.625 / _smoothing_tol * (1.0 - Utility::pow<4>(f_diff / _smoothing_tol));
    case SmootherFunctionType::poly3:
      return (7.0 / 12.0 / _smoothing_tol) * (1.0 - Utility::pow<6>(f_diff / _smoothing_tol));
    default:
      return 0.0;
  }
}

int
MultiParameterPlasticityStressUpdate::lineSearch(Real & res2,
                                                 std::vector<Real> & stress_params,
                                                 Real & gaE,
                                                 const std::vector<Real> & trial_stress_params,
                                                 yieldAndFlow & smoothed_q,
                                                 const std::vector<Real> & intnl_ok,
                                                 std::vector<Real> & intnl,
                                                 std::vector<Real> & rhs,
                                                 Real & linesearch_needed) const
{
  const Real res2_old = res2;
  const std::vector<Real> sp_params_old = stress_params;
  const Real gaE_old = gaE;
  const std::vector<Real> delta_nr_params = rhs;

  Real lam = 1.0;                     // line-search parameter
  const Real lam_min = 1E-10;         // minimum value of lam allowed
  const Real slope = -2.0 * res2_old; // "Numerical Recipes" uses -b*A*x, in order to check for
                                      // roundoff, but i hope the nrStep would warn if there were
                                      // problems
  Real tmp_lam;                       // cached value of lam used in quadratic & cubic line search
  Real f2 = res2_old; // cached value of f = residual2 used in the cubic in the line search
  Real lam2 = lam;    // cached value of lam used in the cubic in the line search

  while (true)
  {
    // update variables using the current line-search parameter
    for (unsigned i = 0; i < _num_sp; ++i)
      stress_params[i] = sp_params_old[i] + lam * delta_nr_params[i];
    gaE = gaE_old + lam * delta_nr_params[_num_sp];

    // and internal parameters
    setIntnlValuesV(trial_stress_params, stress_params, intnl_ok, intnl);

    smoothed_q = smoothAllQuantities(stress_params, intnl);

    // update rhs for next-time through
    calculateRHS(trial_stress_params, stress_params, gaE, smoothed_q, rhs);
    res2 = calculateRes2(rhs);

    // do the line-search
    if (res2 < res2_old + 1E-4 * lam * slope)
      break;
    else if (lam < lam_min)
      return 1;
    else if (lam == 1.0)
    {
      // model as a quadratic
      tmp_lam = -0.5 * slope / (res2 - res2_old - slope);
    }
    else
    {
      // model as a cubic
      const Real rhs1 = res2 - res2_old - lam * slope;
      const Real rhs2 = f2 - res2_old - lam2 * slope;
      const Real a = (rhs1 / Utility::pow<2>(lam) - rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
      const Real b =
          (-lam2 * rhs1 / Utility::pow<2>(lam) + lam * rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
      if (a == 0.0)
        tmp_lam = -slope / (2.0 * b);
      else
      {
        const Real disc = Utility::pow<2>(b) - 3.0 * a * slope;
        if (disc < 0)
          tmp_lam = 0.5 * lam;
        else if (b <= 0)
          tmp_lam = (-b + std::sqrt(disc)) / (3.0 * a);
        else
          tmp_lam = -slope / (b + std::sqrt(disc));
      }
      if (tmp_lam > 0.5 * lam)
        tmp_lam = 0.5 * lam;
    }
    lam2 = lam;
    f2 = res2;
    lam = std::max(tmp_lam, 0.1 * lam);
  }

  if (lam < 1.0)
    linesearch_needed = 1.0;
  return 0;
}

int
MultiParameterPlasticityStressUpdate::nrStep(const yieldAndFlow & smoothed_q,
                                             const std::vector<Real> & trial_stress_params,
                                             const std::vector<Real> & stress_params,
                                             const std::vector<Real> & intnl,
                                             Real gaE,
                                             std::vector<Real> & rhs) const
{
  std::vector<std::vector<Real>> dintnl(_num_intnl, std::vector<Real>(_num_sp));
  setIntnlDerivativesV(trial_stress_params, stress_params, intnl, dintnl);

  std::vector<double> jac((_num_sp + 1) * (_num_sp + 1));
  dnRHSdVar(smoothed_q, dintnl, stress_params, gaE, jac);

  // use LAPACK to solve the linear system
  const PetscBLASInt nrhs = 1;
  std::vector<PetscBLASInt> ipiv(_num_sp + 1);
  PetscBLASInt info;
  const PetscBLASInt gesv_num_rhs = _num_sp + 1;
  LAPACKgesv_(
      &gesv_num_rhs, &nrhs, &jac[0], &gesv_num_rhs, &ipiv[0], &rhs[0], &gesv_num_rhs, &info);
  return info;
}

void
MultiParameterPlasticityStressUpdate::errorHandler(const std::string & message) const
{
  throw MooseException(message);
}

void
MultiParameterPlasticityStressUpdate::initializeReturnProcess()
{
}

void
MultiParameterPlasticityStressUpdate::finalizeReturnProcess(
    const RankTwoTensor & /*rotation_increment*/)
{
}

void
MultiParameterPlasticityStressUpdate::preReturnMapV(
    const std::vector<Real> & /*trial_stress_params*/,
    const RankTwoTensor & /*stress_trial*/,
    const std::vector<Real> & /*intnl_old*/,
    const std::vector<Real> & /*yf*/,
    const RankFourTensor & /*Eijkl*/)
{
}

Real
MultiParameterPlasticityStressUpdate::yieldF(const std::vector<Real> & stress_params,
                                             const std::vector<Real> & intnl) const
{
  std::vector<Real> yfs(_num_yf);
  yieldFunctionValuesV(stress_params, intnl, yfs);
  return yieldF(yfs);
}

Real
MultiParameterPlasticityStressUpdate::yieldF(const std::vector<Real> & yfs) const
{
  Real yf = yfs[0];
  for (std::size_t i = 1; i < yfs.size(); ++i)
    if (yf >= yfs[i] + _smoothing_tol)
      // no smoothing is needed, and yf is the biggest yield function
      continue;
    else if (yfs[i] >= yf + _smoothing_tol)
      // no smoothing is needed, and yfs[i] is the biggest yield function
      yf = yfs[i];
    else
      yf = 0.5 * (yf + yfs[i] + _smoothing_tol) + ismoother(yf - yfs[i]);
  return yf;
}

void
MultiParameterPlasticityStressUpdate::initializeVarsV(const std::vector<Real> & trial_stress_params,
                                                      const std::vector<Real> & intnl_old,
                                                      std::vector<Real> & stress_params,
                                                      Real & gaE,
                                                      std::vector<Real> & intnl) const
{
  gaE = 0.0;
  std::copy(trial_stress_params.begin(), trial_stress_params.end(), stress_params.begin());
  std::copy(intnl_old.begin(), intnl_old.end(), intnl.begin());
}

void
MultiParameterPlasticityStressUpdate::consistentTangentOperatorV(
    const RankTwoTensor & stress_trial,
    const std::vector<Real> & /*trial_stress_params*/,
    const RankTwoTensor & stress,
    const std::vector<Real> & /*stress_params*/,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & elasticity_tensor,
    bool compute_full_tangent_operator,
    const std::vector<std::vector<Real>> & dvar_dtrial,
    RankFourTensor & cto)
{
  cto = elasticity_tensor;
  if (!compute_full_tangent_operator)
    return;

  const Real ga = gaE / _En;

  const std::vector<RankTwoTensor> dsp = dstress_param_dstress(stress);
  const std::vector<RankTwoTensor> dsp_trial = dstress_param_dstress(stress_trial);

  for (unsigned a = 0; a < _num_sp; ++a)
  {
    for (unsigned b = 0; b < _num_sp; ++b)
    {
      const RankTwoTensor t = elasticity_tensor * dsp_trial[a];
      RankTwoTensor s = _Cij[b][a] * dsp[b];
      for (unsigned c = 0; c < _num_sp; ++c)
        s -= dsp[b] * _Cij[b][c] * dvar_dtrial[c][a];
      s = elasticity_tensor * s;
      cto -= s.outerProduct(t);
    }
  }

  const std::vector<RankFourTensor> d2sp = d2stress_param_dstress(stress);
  RankFourTensor Tijab;
  for (unsigned i = 0; i < _num_sp; ++i)
    Tijab += smoothed_q.dg[i] * d2sp[i];
  Tijab = ga * elasticity_tensor * Tijab;

  RankFourTensor inv = RankFourTensor(RankFourTensor::initIdentityFour) + Tijab;
  try
  {
    inv = inv.transposeMajor().invSymm();
  }
  catch (const MooseException & e)
  {
    // Cannot form the inverse, so probably at some degenerate place in stress space.
    // Just return with the "best estimate" of the cto.
    mooseWarning("MultiParameterPlasticityStressUpdate: Cannot invert 1+T in consistent tangent "
                 "operator computation at quadpoint ",
                 _qp,
                 " of element ",
                 _current_elem->id());
    return;
  }

  cto = (cto.transposeMajor() * inv).transposeMajor();
}

void
MultiParameterPlasticityStressUpdate::setInelasticStrainIncrementAfterReturn(
    const RankTwoTensor & /*stress_trial*/,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & /*elasticity_tensor*/,
    const RankTwoTensor & returned_stress,
    RankTwoTensor & inelastic_strain_increment) const
{
  const std::vector<RankTwoTensor> dsp_dstress = dstress_param_dstress(returned_stress);
  inelastic_strain_increment = RankTwoTensor();
  for (unsigned i = 0; i < _num_sp; ++i)
    inelastic_strain_increment += smoothed_q.dg[i] * dsp_dstress[i];
  inelastic_strain_increment *= gaE / _En;
}

Real
MultiParameterPlasticityStressUpdate::calculateRes2(const std::vector<Real> & rhs) const
{
  Real res2 = 0.0;
  for (const auto & r : rhs)
    res2 += r * r;
  return res2;
}

void
MultiParameterPlasticityStressUpdate::calculateRHS(const std::vector<Real> & trial_stress_params,
                                                   const std::vector<Real> & stress_params,
                                                   Real gaE,
                                                   const yieldAndFlow & smoothed_q,
                                                   std::vector<Real> & rhs) const
{
  const Real ga = gaE / _En;
  for (unsigned i = 0; i < _num_sp; ++i)
  {
    rhs[i] = stress_params[i] - trial_stress_params[i];
    for (unsigned j = 0; j < _num_sp; ++j)
      rhs[i] += ga * _Eij[i][j] * smoothed_q.dg[j];
  }
  rhs[_num_sp] = smoothed_q.f;
}

void
MultiParameterPlasticityStressUpdate::dnRHSdVar(const yieldAndFlow & smoothed_q,
                                                const std::vector<std::vector<Real>> & dintnl,
                                                const std::vector<Real> & /*stress_params*/,
                                                Real gaE,
                                                std::vector<double> & jac) const
{
  for (auto & jac_entry : jac)
    jac_entry = 0.0;

  const Real ga = gaE / _En;

  unsigned ind = 0;
  for (unsigned var = 0; var < _num_sp; ++var)
  {
    for (unsigned rhs = 0; rhs < _num_sp; ++rhs)
    {
      if (var == rhs)
        jac[ind] -= 1.0;
      for (unsigned j = 0; j < _num_sp; ++j)
      {
        jac[ind] -= ga * _Eij[rhs][j] * smoothed_q.d2g[j][var];
        for (unsigned k = 0; k < _num_intnl; ++k)
          jac[ind] -= ga * _Eij[rhs][j] * smoothed_q.d2g_di[j][k] * dintnl[k][var];
      }
      ind++;
    }
    // now rhs = _num_sp (that is, the yield function)
    jac[ind] -= smoothed_q.df[var];
    for (unsigned k = 0; k < _num_intnl; ++k)
      jac[ind] -= smoothed_q.df_di[k] * dintnl[k][var];
    ind++;
  }

  // now var = _num_sp (that is, gaE)
  for (unsigned rhs = 0; rhs < _num_sp; ++rhs)
  {
    for (unsigned j = 0; j < _num_sp; ++j)
      jac[ind] -= (1.0 / _En) * _Eij[rhs][j] * smoothed_q.dg[j];
    ind++;
  }
  // now rhs = _num_sp (that is, the yield function)
  jac[ind] = 0.0;
}

void
MultiParameterPlasticityStressUpdate::dVardTrial(bool elastic_only,
                                                 const std::vector<Real> & trial_stress_params,
                                                 const std::vector<Real> & stress_params,
                                                 Real gaE,
                                                 const std::vector<Real> & intnl,
                                                 const yieldAndFlow & smoothed_q,
                                                 Real step_size,
                                                 bool compute_full_tangent_operator,
                                                 std::vector<std::vector<Real>> & dvar_dtrial) const
{
  if (!_fe_problem.currentlyComputingJacobian())
    return;

  if (!compute_full_tangent_operator)
    return;

  if (elastic_only)
  {
    // no change to gaE, and all off-diag stuff remains unchanged from previous step
    for (unsigned v = 0; v < _num_sp; ++v)
      dvar_dtrial[v][v] += step_size;
    return;
  }

  const Real ga = gaE / _En;

  std::vector<std::vector<Real>> dintnl(_num_intnl, std::vector<Real>(_num_sp));
  setIntnlDerivativesV(trial_stress_params, stress_params, intnl, dintnl);

  // rhs is described elsewhere, the following are changes in rhs wrt the trial_stress_param
  // values
  // In the following we use d(intnl)/d(trial variable) = - d(intnl)/d(variable)
  std::vector<Real> rhs_cto((_num_sp + 1) * _num_sp);

  unsigned ind = 0;
  for (unsigned a = 0; a < _num_sp; ++a)
  {
    // change in RHS[b] wrt changes in stress_param_trial[a]
    for (unsigned b = 0; b < _num_sp; ++b)
    {
      if (a == b)
        rhs_cto[ind] -= 1.0;
      for (unsigned j = 0; j < _num_sp; ++j)
        for (unsigned k = 0; k < _num_intnl; ++k)
          rhs_cto[ind] -= ga * _Eij[b][j] * smoothed_q.d2g_di[j][k] * dintnl[k][a];
      ind++;
    }
    // now b = _num_sp (that is, the yield function)
    for (unsigned k = 0; k < _num_intnl; ++k)
      rhs_cto[ind] -= smoothed_q.df_di[k] * dintnl[k][a];
    ind++;
  }

  // jac = d(-rhs)/d(var)
  std::vector<double> jac((_num_sp + 1) * (_num_sp + 1));
  dnRHSdVar(smoothed_q, dintnl, stress_params, gaE, jac);

  std::vector<PetscBLASInt> ipiv(_num_sp + 1);
  PetscBLASInt info;
  const PetscBLASInt gesv_num_rhs = _num_sp + 1;
  const PetscBLASInt gesv_num_pq = _num_sp;
  LAPACKgesv_(&gesv_num_rhs,
              &gesv_num_pq,
              &jac[0],
              &gesv_num_rhs,
              &ipiv[0],
              &rhs_cto[0],
              &gesv_num_rhs,
              &info);
  if (info != 0)
    errorHandler("MultiParameterPlasticityStressUpdate: PETSC LAPACK gsev routine returned with "
                 "error code " +
                 Moose::stringify(info));

  ind = 0;
  std::vector<std::vector<Real>> dvarn_dtrialn(_num_sp + 1, std::vector<Real>(_num_sp, 0.0));
  for (unsigned spt = 0; spt < _num_sp; ++spt) // loop over trial stress-param variables
  {
    for (unsigned v = 0; v < _num_sp; ++v) // loop over variables in NR procedure
    {
      dvarn_dtrialn[v][spt] = rhs_cto[ind];
      ind++;
    }
    // the final NR variable is gaE
    dvarn_dtrialn[_num_sp][spt] = rhs_cto[ind];
    ind++;
  }

  const std::vector<std::vector<Real>> dvar_dtrial_old = dvar_dtrial;

  for (unsigned v = 0; v < _num_sp; ++v) // loop over variables in NR procedure
  {
    for (unsigned spt = 0; spt < _num_sp; ++spt) // loop over trial stress-param variables
    {
      dvar_dtrial[v][spt] = step_size * dvarn_dtrialn[v][spt];
      for (unsigned a = 0; a < _num_sp; ++a)
        dvar_dtrial[v][spt] += dvarn_dtrialn[v][a] * dvar_dtrial_old[a][spt];
    }
  }
  // for gaE the formulae are a little different
  const unsigned v = _num_sp;
  for (unsigned spt = 0; spt < _num_sp; ++spt)
  {
    dvar_dtrial[v][spt] += step_size * dvarn_dtrialn[v][spt]; // note +=
    for (unsigned a = 0; a < _num_sp; ++a)
      dvar_dtrial[v][spt] += dvarn_dtrialn[v][a] * dvar_dtrial_old[a][spt];
  }
}

bool
MultiParameterPlasticityStressUpdate::precisionLoss(const std::vector<Real> & solution,
                                                    const std::vector<Real> & stress_params,
                                                    Real gaE) const
{
  if (std::abs(solution[_num_sp]) > 1E-13 * std::abs(gaE))
    return false;
  for (unsigned i = 0; i < _num_sp; ++i)
    if (std::abs(solution[i]) > 1E-13 * std::abs(stress_params[i]))
      return false;
  return true;
}
