/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TwoParameterPlasticityStressUpdate.h"

#include "Conversion.h"      // for stringify
#include "libmesh/utility.h" // for Utility::pow

template <>
InputParameters
validParams<TwoParameterPlasticityStressUpdate>()
{
  InputParameters params = validParams<StressUpdateBase>();
  params.addClassDescription("Return-map and Jacobian algorithms for (P, Q) plastic models");
  params.addParam<std::string>("name_prepender",
                               "Optional parameter that allows the user to define "
                               "multiple plastic models on the same block, and the "
                               "plastic_internal_parameter, plastic_yield_function, "
                               "plastic_NR_iterations and plastic_linesearch_needed Material "
                               "Properties will be prepended by this string");
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
  params.addParam<std::vector<Real>>(
      "admissible_stress",
      std::vector<Real>{0.0, 0.0},
      "A single admissible value of the value of (p, q) for internal parameters = 0.  This is used "
      "to initialise the return-mapping algorithm during the first nonlinear iteration");
  return params;
}

TwoParameterPlasticityStressUpdate::TwoParameterPlasticityStressUpdate(
    const InputParameters & parameters, unsigned num_yf, unsigned num_intnl)
  : StressUpdateBase(parameters),
    _num_yf(num_yf),
    _num_intnl(num_intnl),
    _name_prepender(isParamValid("name_prepender") ? getParam<std::string>("name_prepender") + "_"
                                                   : ""),
    _max_nr_its(getParam<unsigned>("max_NR_iterations")),
    _perform_finite_strain_rotations(getParam<bool>("perform_finite_strain_rotations")),
    _smoothing_tol(getParam<Real>("smoothing_tol")),
    _f_tol(getParam<Real>("yield_function_tol")),
    _f_tol2(Utility::pow<2>(getParam<Real>("yield_function_tol"))),
    _min_step_size(getParam<Real>("min_step_size")),
    _step_one(declareRestartableData<bool>("step_one", true)),
    _warn_about_precision_loss(getParam<bool>("warn_about_precision_loss")),

    _plastic_strain(declareProperty<RankTwoTensor>(_name_prepender + "plastic_strain")),
    _plastic_strain_old(getMaterialPropertyOld<RankTwoTensor>(_name_prepender + "plastic_strain")),
    _intnl(declareProperty<std::vector<Real>>(_name_prepender + "plastic_internal_parameter")),
    _intnl_old(
        getMaterialPropertyOld<std::vector<Real>>(_name_prepender + "plastic_internal_parameter")),
    _yf(declareProperty<std::vector<Real>>(_name_prepender + "plastic_yield_function")),
    _iter(declareProperty<Real>(_name_prepender +
                                "plastic_NR_iterations")), // this is really an unsigned int, but
                                                           // for visualisation i convert it to Real
    _linesearch_needed(
        declareProperty<Real>(_name_prepender + "plastic_linesearch_needed")), // this is really a
                                                                               // boolean, but for
                                                                               // visualisation i
                                                                               // convert it to Real

    _p_trial(0.0),
    _q_trial(0.0),
    _intnl_ok(_num_intnl),
    _dintnl(_num_intnl),
    _Epp(0.0),
    _Eqq(0.0),

    _dgaE_dpt(0.0),
    _dp_dpt(0.0),
    _dq_dpt(0.0),
    _dgaE_dqt(0.0),
    _dp_dqt(0.0),
    _dq_dqt(0.0),
    _all_q(_num_yf),

    _pq_ok(getParam<std::vector<Real>>("admissible_stress"))
{
  for (unsigned i = 0; i < _num_intnl; ++i)
    _dintnl[i].resize(_num_pq);
  for (unsigned i = 0; i < _num_yf; ++i)
    _all_q[i] = f_and_derivs(_num_pq, _num_intnl);
  if (_pq_ok.size() != 2)
    mooseError(
        "TwoParameterPlasticityStressUpdate: admissible_stress parameter must be two numbers");
}

void
TwoParameterPlasticityStressUpdate::initQpStatefulProperties()
{
  _plastic_strain[_qp].zero();
  _intnl[_qp].assign(_num_intnl, 0);
  _yf[_qp].assign(_num_yf, 0);
  _iter[_qp] = 0.0;
  _linesearch_needed[_qp] = 0.0;
}

void
TwoParameterPlasticityStressUpdate::propagateQpStatefulProperties()
{
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
  for (unsigned i = 0; i < _num_intnl; ++i)
    _intnl[_qp][i] = _intnl_old[_qp][i];
}

void
TwoParameterPlasticityStressUpdate::updateState(RankTwoTensor & strain_increment,
                                                RankTwoTensor & inelastic_strain_increment,
                                                const RankTwoTensor & rotation_increment,
                                                RankTwoTensor & stress_new,
                                                const RankTwoTensor & stress_old,
                                                const RankFourTensor & elasticity_tensor,
                                                const RankTwoTensor & /*elastic_strain_old*/,
                                                bool compute_full_tangent_operator,
                                                RankFourTensor & tangent_operator)
{
  initialiseReturnProcess();

  if (_t_step >= 2)
    _step_one = false;

  // initially assume an elastic deformation
  for (unsigned i = 0; i < _num_intnl; ++i)
    _intnl[_qp][i] = _intnl_old[_qp][i];
  _iter[_qp] = 0.0;
  _linesearch_needed[_qp] = 0.0;

  computePQ(stress_new, _p_trial, _q_trial);
  yieldFunctionValues(_p_trial, _q_trial, _intnl[_qp], _yf[_qp]);

  /* Need to consider smoothing_tol, not just yf<=0, because
   * some yield functions might mix.  However, if some yield
   * functions are -smoothing_tol <= yf, then the smoothed
   * yield function must be computed and checked vs f_tol
   */
  bool elastic = true;
  for (auto yf : _yf[_qp])
    if (yf > -_smoothing_tol)
      elastic = false;
  if (!elastic && yieldF(_p_trial, _q_trial, _intnl[_qp]) <= _f_tol)
    elastic = true;
  if (elastic)
  {
    _plastic_strain[_qp] = _plastic_strain_old[_qp];
    if (_fe_problem.currentlyComputingJacobian())
    {
      inelastic_strain_increment.zero();
      tangent_operator = elasticity_tensor;
    }
    return;
  }

  const RankTwoTensor stress_trial = stress_new;
  /* The trial stress must be inadmissible
   * so we need to return to the yield surface.  The following
   * equations must be satisfied.
   *
   * 0 = f(p, q, intnl)                  = rhs[0]
   * 0 = p - p^trial + Epp * ga * dg/dp  = rhs[1]
   * 0 = q - q^trial + Eqq * ga * dg/dq  = rhs[2]
   * Equations defining intnl parameters as functions of (p, q, p_trial, q_trial, intnl_old)
   *
   * The unknowns are p, q, ga, and intnl.
   * I find it convenient to solve the first three equations for p, q and ga*Epp=gaE,
   * while substituting the "intnl parameters" equations into these during the solve process
   */

  preReturnMap(_p_trial, _q_trial, stress_new, _intnl_old[_qp], _yf[_qp], elasticity_tensor);
  setEppEqq(elasticity_tensor, _Epp, _Eqq);

  // values of p, q  and intnl we know to be admissible
  Real p_ok;
  Real q_ok;
  computePQ(stress_old, p_ok, q_ok);
  std::copy(_intnl_old[_qp].begin(), _intnl_old[_qp].end(), _intnl_ok.begin());
  // note that _intnl[_qp] is the "running" intnl variable that gets updated every NR iteration
  // if (isParamValid("initial_stress") && _step_one)
  if (_step_one)
  {
    // the initial stress might be inadmissible
    p_ok = _pq_ok[0];
    q_ok = _pq_ok[1];
  }

  _dgaE_dpt = 0.0;
  _dp_dpt = 0.0;
  _dq_dpt = 0.0;
  _dgaE_dqt = 0.0;
  _dp_dqt = 0.0;
  _dq_dqt = 0.0;

  // Return-map problem: must apply the following changes in p and q, and find the returned p and q.
  const Real del_p = _p_trial - p_ok;
  const Real del_q = _q_trial - q_ok;

  Real step_taken =
      0.0; // amount of (del_p, del_q) that we've applied and the return-map problem has succeeded
  Real step_size = 1.0; // potentially can apply (del_p, del_q) in substeps
  Real gaE_total = 0.0;

  // current values of the yield function, derivatives, etc
  f_and_derivs smoothed_q;

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
    _p_trial = p_ok + step_size * del_p;
    _q_trial = q_ok + step_size * del_q;

    // initialise (p, q, gaE)
    Real p = _p_trial;
    Real q = _q_trial;
    Real gaE = 0.0;

    // flags indicating failure of newton-raphson and line-search
    int nr_failure = 0;
    int ls_failure = 0;

    // NR iterations taken in this substep
    Real step_iter = 0.0;

    // The residual-squared for the line-search
    Real res2 = 0.0;

    if (step_size < 1.0 && yieldF(_p_trial, _q_trial, _intnl[_qp]) <= _f_tol)
      // This is an elastic step
      // The "step_size < 1.0" in above condition is for efficiency: we definitely
      // know that this is a plastic step if step_size = 1.0
      smoothed_q_calculated = false;
    else
    {
      // this is a plastic step

      // initialise p, q and gaE using a good guess based on the non-smoothed situation
      initialiseVars(_p_trial, _q_trial, _intnl_ok, p, q, gaE, _intnl[_qp]);
      smoothed_q = smoothAllQuantities(p, q, _intnl[_qp]);
      smoothed_q_calculated = true;
      res2 = calculateRHS(_p_trial, _q_trial, p, q, gaE, smoothed_q);

      // Perform a Newton-Raphson with linesearch to get p, q, gaE, and also smoothed_q
      while (res2 > _f_tol2 && step_iter < (float)_max_nr_its && nr_failure == 0 && ls_failure == 0)
      {
        // solve the linear system and store the answer (the "updates") in _rhs
        nr_failure = nrStep(smoothed_q, _p_trial, _q_trial, p, q, gaE);
        if (nr_failure != 0)
          break;

        // check for precision loss
        if (std::abs(_rhs[0]) <= 1E-13 * std::abs(gaE) &&
            std::abs(_rhs[1]) <= 1E-13 * std::abs(p) && std::abs(_rhs[2]) <= 1E-13 * std::abs(q))
        {
          if (_warn_about_precision_loss)
            Moose::err << "TwoParameterPlasticityStressUpdate: precision-loss in element "
                       << _current_elem->id() << " quadpoint=" << _qp << " p=" << p << " q=" << q
                       << " gaE=" << gaE << "\n";
          res2 = 0.0;
          break;
        }
        // apply (parts of) the updates, re-calculate smoothed_q, and res2
        ls_failure = lineSearch(res2, gaE, p, q, _p_trial, _q_trial, smoothed_q, _intnl_ok);
        step_iter++;
      }
    }

    if (res2 <= _f_tol2 && step_iter < (float)_max_nr_its && nr_failure == 0 && ls_failure == 0 &&
        gaE >= 0.0)
    {
      // this Newton-Raphson worked fine, or this was an elastic step
      p_ok = p;
      q_ok = q;
      step_taken += step_size;
      gaE_total += gaE;
      setIntnlValues(_p_trial, _q_trial, p_ok, q_ok, _intnl_ok, _intnl[_qp]);
      std::copy(_intnl[_qp].begin(), _intnl[_qp].end(), _intnl_ok.begin());
      // calculate dp/dp_trial, dp/dq_trial, etc, for Jacobian
      dVardTrial(!smoothed_q_calculated,
                 _p_trial,
                 _q_trial,
                 p_ok,
                 q_ok,
                 gaE,
                 _intnl_ok,
                 smoothed_q,
                 step_size,
                 compute_full_tangent_operator);
      if (step_iter > _iter[_qp])
        _iter[_qp] = step_iter;
      step_size *= 1.1;
    }
    else
    {
      // Newton-Raphson + line-search process failed
      std::copy(_intnl_ok.begin(), _intnl_ok.end(), _intnl[_qp].begin());
      step_size *= 0.5;
    }
  }

  if (step_size < _min_step_size)
    errorHandler("TwoParameterPlasticityStressUpdate: Minimum step-size violated");

  // success!
  finalizeReturnProcess(rotation_increment);
  yieldFunctionValues(p_ok, q_ok, _intnl[_qp], _yf[_qp]);

  if (!smoothed_q_calculated)
    smoothed_q = smoothAllQuantities(p_ok, q_ok, _intnl[_qp]);

  setStressAfterReturn(
      stress_trial, p_ok, q_ok, gaE_total, _intnl[_qp], smoothed_q, elasticity_tensor, stress_new);

  setInelasticStrainIncrementAfterReturn(stress_trial,
                                         gaE_total,
                                         smoothed_q,
                                         elasticity_tensor,
                                         stress_new,
                                         inelastic_strain_increment);

  strain_increment = strain_increment - inelastic_strain_increment;
  _plastic_strain[_qp] = _plastic_strain_old[_qp] + inelastic_strain_increment;

  consistentTangentOperator(stress_trial,
                            _p_trial,
                            _q_trial,
                            stress_new,
                            p_ok,
                            q_ok,
                            gaE_total,
                            smoothed_q,
                            elasticity_tensor,
                            compute_full_tangent_operator,
                            tangent_operator);
}

void
TwoParameterPlasticityStressUpdate::dVardTrial(bool elastic_only,
                                               Real p_trial,
                                               Real q_trial,
                                               Real p,
                                               Real q,
                                               Real gaE,
                                               const std::vector<Real> & intnl,
                                               const f_and_derivs & smoothed_q,
                                               Real step_size,
                                               bool compute_full_tangent_operator)
{
  if (!_fe_problem.currentlyComputingJacobian())
    return;

  if (!compute_full_tangent_operator)
    return;

  if (elastic_only)
  {
    // no change to gaE, and dp_dqt and dq_dpt are unchanged from previous step
    _dp_dpt = step_size + _dp_dpt;
    _dq_dqt = step_size + _dq_dqt;
    return;
  }

  setIntnlDerivatives(p_trial, q_trial, p, q, intnl, _dintnl);

  // _rhs is defined above, the following are changes in rhs wrt the trial p and q values
  // In the following we use d(intnl)/d(trial variable) = - d(intnl)/d(variable)
  std::array<Real, _num_pq * _num_rhs> rhs_cto{{0.0}};

  // change in p_trial
  for (unsigned i = 0; i < _num_intnl; ++i)
    rhs_cto[0] -= smoothed_q.df_di[i] * _dintnl[i][0];
  rhs_cto[1] = -1.0;
  for (unsigned i = 0; i < _num_intnl; ++i)
    rhs_cto[1] -= gaE * smoothed_q.d2g_di[0][i] * _dintnl[i][0];
  for (unsigned i = 0; i < _num_intnl; ++i)
    rhs_cto[2] -= _Eqq * gaE / _Epp * smoothed_q.d2g_di[1][i] * _dintnl[i][0];

  // change in q_trial
  for (unsigned i = 0; i < _num_intnl; ++i)
    rhs_cto[3] -= smoothed_q.df_di[i] * _dintnl[i][1];
  for (unsigned i = 0; i < _num_intnl; ++i)
    rhs_cto[4] -= gaE * smoothed_q.d2g_di[0][i] * _dintnl[i][1];
  rhs_cto[5] = -1.0;
  for (unsigned i = 0; i < _num_intnl; ++i)
    rhs_cto[5] -= _Eqq * gaE / _Epp * smoothed_q.d2g_di[1][i] * _dintnl[i][1];

  // jac = d(-rhs)/d(var), where var[0] = gaE, var[1] = p, var[2] = q.
  std::array<double, _num_rhs * _num_rhs> jac;
  dnRHSdVar(smoothed_q, _dintnl, gaE, jac);

  std::array<int, _num_rhs> ipiv;
  int info;
  const int gesv_num_rhs = _num_rhs;
  const int gesv_num_pq = _num_pq;
  LAPACKgesv_(&gesv_num_rhs,
              &gesv_num_pq,
              &jac[0],
              &gesv_num_rhs,
              &ipiv[0],
              &rhs_cto[0],
              &gesv_num_rhs,
              &info);
  if (info != 0)
    errorHandler(
        "TwoParameterPlasticityStressUpdate: PETSC LAPACK gsev routine returned with error code " +
        Moose::stringify(info));

  const Real dgaEn_dptn = rhs_cto[0];
  const Real dpn_dptn = rhs_cto[1];
  const Real dqn_dptn = rhs_cto[2];
  const Real dgaEn_dqtn = rhs_cto[3];
  const Real dpn_dqtn = rhs_cto[4];
  const Real dqn_dqtn = rhs_cto[5];

  const Real dp_dpt_old = _dp_dpt;
  const Real dq_dpt_old = _dq_dpt;
  const Real dp_dqt_old = _dp_dqt;
  const Real dq_dqt_old = _dq_dqt;

  _dgaE_dpt += dgaEn_dptn * (step_size + dp_dpt_old) + dgaEn_dqtn * dq_dpt_old;
  _dp_dpt = dpn_dptn * (step_size + dp_dpt_old) + dpn_dqtn * dq_dpt_old;
  _dq_dpt = dqn_dptn * (step_size + dp_dpt_old) + dqn_dqtn * dq_dpt_old;
  _dgaE_dqt += dgaEn_dqtn * (step_size + dq_dqt_old) + dgaEn_dptn * dp_dqt_old;
  _dp_dqt = dpn_dqtn * (step_size + dq_dqt_old) + dpn_dptn * dp_dqt_old;
  _dq_dqt = dqn_dqtn * (step_size + dq_dqt_old) + dqn_dptn * dp_dqt_old;
}

TwoParameterPlasticityStressUpdate::f_and_derivs
TwoParameterPlasticityStressUpdate::smoothAllQuantities(Real p,
                                                        Real q,
                                                        const std::vector<Real> & intnl)
{
  for (unsigned i = _all_q.size(); i < _num_yf; ++i)
    _all_q.push_back(f_and_derivs(_num_pq, _num_intnl));

  computeAllQ(p, q, intnl, _all_q);

  std::sort(_all_q.begin(), _all_q.end());

  /* This is the key to my smoothing strategy.  While the two
   * biggest yield functions are closer to each other than
   * _smoothing_tol, make a linear combination of them:
   * _all_q[num - 2].f = the second-biggest yield function
   * = _all_q[num - 1].f + ismoother(_all_q[num - 2].f - _all_q[num - 1].f);
   * = biggest yield function + ismoother(second-biggest - biggest)
   * Then pop off the biggest yield function, and repeat this
   * strategy.
   * Of course all the derivatives must also be smoothed.
   * I assume that d(flow potential)/dstress gets smoothed by the *yield function*, viz
   * d(second-biggest-g) = d(biggest-g) + smoother(second-biggest-f -
   * biggest-f)*(d(second-biggest-g) - d(biggest-g))
   * Only time will tell whether this is a good strategy.
   */
  unsigned num = _all_q.size();
  while (num > 1 && _all_q[num - 1].f < _all_q[num - 2].f + _smoothing_tol)
  {
    const Real ism = ismoother(_all_q[num - 2].f - _all_q[num - 1].f);
    const Real sm = smoother(_all_q[num - 2].f - _all_q[num - 1].f);
    const Real dsm = dsmoother(_all_q[num - 2].f - _all_q[num - 1].f);
    for (unsigned i = 0; i < _num_pq; ++i)
    {
      for (unsigned j = 0; j < _num_pq; ++j)
        _all_q[num - 2].d2g[i][j] = _all_q[num - 1].d2g[i][j] +
                                    dsm * (_all_q[num - 2].df[j] - _all_q[num - 1].df[j]) *
                                        (_all_q[num - 2].dg[i] - _all_q[num - 1].dg[i]) +
                                    sm * (_all_q[num - 2].d2g[i][j] - _all_q[num - 1].d2g[i][j]);
      for (unsigned j = 0; j < _num_intnl; ++j)
        _all_q[num - 2].d2g_di[i][j] =
            _all_q[num - 1].d2g_di[i][j] +
            dsm * (_all_q[num - 2].df_di[j] - _all_q[num - 1].df_di[j]) *
                (_all_q[num - 2].dg[i] - _all_q[num - 1].dg[i]) +
            sm * (_all_q[num - 2].d2g_di[i][j] - _all_q[num - 1].d2g_di[i][j]);
    }
    for (unsigned i = 0; i < _num_pq; ++i)
    {
      _all_q[num - 2].dg[i] =
          _all_q[num - 1].dg[i] + sm * (_all_q[num - 2].dg[i] - _all_q[num - 1].dg[i]);
      _all_q[num - 2].df[i] =
          _all_q[num - 1].df[i] + sm * (_all_q[num - 2].df[i] - _all_q[num - 1].df[i]);
    }
    for (unsigned i = 0; i < _num_intnl; ++i)
      _all_q[num - 2].df_di[i] =
          _all_q[num - 1].df_di[i] + sm * (_all_q[num - 2].df_di[i] - _all_q[num - 1].df_di[i]);
    _all_q[num - 2].f = _all_q[num - 1].f + ism;
    _all_q.pop_back();
    num = _all_q.size();
  }
  return _all_q.back();
}

Real
TwoParameterPlasticityStressUpdate::ismoother(Real f_diff) const
{
  mooseAssert(f_diff <= 0.0,
              "TwoParameterPlasticityStressUpdate: ismoother called with positive argument");
  if (f_diff <= -_smoothing_tol)
    return 0.0;
  return 0.5 * (f_diff + _smoothing_tol) -
         _smoothing_tol / M_PI * std::cos(0.5 * M_PI * f_diff / _smoothing_tol);
}

Real
TwoParameterPlasticityStressUpdate::smoother(Real f_diff) const
{
  if (f_diff >= _smoothing_tol)
    return 1.0;
  else if (f_diff <= -_smoothing_tol)
    return 0.0;
  return 0.5 * (1.0 + std::sin(f_diff * M_PI * 0.5 / _smoothing_tol));
}

Real
TwoParameterPlasticityStressUpdate::dsmoother(Real f_diff) const
{
  if (f_diff >= _smoothing_tol)
    return 0.0;
  else if (f_diff <= -_smoothing_tol)
    return 0.0;
  return 0.25 * M_PI / _smoothing_tol * std::cos(f_diff * M_PI * 0.5 / _smoothing_tol);
}

int
TwoParameterPlasticityStressUpdate::lineSearch(Real & res2,
                                               Real & gaE,
                                               Real & p,
                                               Real & q,
                                               Real p_trial,
                                               Real q_trial,
                                               f_and_derivs & smoothed_q,
                                               const std::vector<Real> & intnl_ok)
{
  const Real res2_old = res2;
  const Real gaE_old = gaE;
  const Real p_old = p;
  const Real q_old = q;
  const Real de_gaE = _rhs[0];
  const Real de_p = _rhs[1];
  const Real de_q = _rhs[2];

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
    gaE = gaE_old + lam * de_gaE;
    p = p_old + lam * de_p;
    q = q_old + lam * de_q;

    // and internal parameters
    setIntnlValues(_p_trial, _q_trial, p, q, intnl_ok, _intnl[_qp]);

    smoothed_q = smoothAllQuantities(p, q, _intnl[_qp]);

    // update rhs for next-time through
    res2 = calculateRHS(p_trial, q_trial, p, q, gaE, smoothed_q);

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
    _linesearch_needed[_qp] = 1.0;
  return 0;
}

void
TwoParameterPlasticityStressUpdate::dnRHSdVar(const f_and_derivs & smoothed_q,
                                              const std::vector<std::vector<Real>> & dintnl,
                                              Real gaE,
                                              std::array<double, _num_rhs * _num_rhs> & jac) const
{
  // LAPACK gsev stores the matrix in the following way:

  // d(-yieldF)/d(gaE)
  jac[0] = 0;

  // d(-rhs[1])/d(gaE)
  jac[1] = -smoothed_q.dg[0];

  // d(-rhs[2])/d(gaE))
  jac[2] = -_Eqq * smoothed_q.dg[1] / _Epp;

  // d(-yieldF)/d(p)
  jac[3] = -smoothed_q.df[0];
  for (unsigned i = 0; i < _num_intnl; ++i)
    jac[3] -= smoothed_q.df_di[i] * dintnl[i][0];

  // d(-rhs[1])/d(p)
  jac[4] = -1.0 - gaE * smoothed_q.d2g[0][0];
  for (unsigned i = 0; i < _num_intnl; ++i)
    jac[4] -= gaE * smoothed_q.d2g_di[0][i] * dintnl[i][0];

  // d(-rhs[2])/d(p)
  jac[5] = -_Eqq * gaE / _Epp * smoothed_q.d2g[1][0];
  for (unsigned i = 0; i < _num_intnl; ++i)
    jac[5] -= _Eqq * gaE / _Epp * smoothed_q.d2g_di[1][i] * dintnl[i][0];

  // d(-yieldF)/d(q)
  jac[6] = -smoothed_q.df[1];
  for (unsigned i = 0; i < _num_intnl; ++i)
    jac[6] -= smoothed_q.df_di[i] * dintnl[i][1];

  // d(-rhs[1])/d(q)
  jac[7] = -gaE * smoothed_q.d2g[0][1];
  for (unsigned i = 0; i < _num_intnl; ++i)
    jac[7] -= gaE * smoothed_q.d2g_di[0][i] * dintnl[i][1];

  // d(-rhs[2])/d(q)
  jac[8] = -1.0 - _Eqq * gaE / _Epp * smoothed_q.d2g[1][1];
  for (unsigned i = 0; i < _num_intnl; ++i)
    jac[8] -= _Eqq * gaE / _Epp * smoothed_q.d2g_di[1][i] * dintnl[i][1];
}

int
TwoParameterPlasticityStressUpdate::nrStep(
    const f_and_derivs & smoothed_q, Real p_trial, Real q_trial, Real p, Real q, Real gaE)
{
  setIntnlDerivatives(p_trial, q_trial, p, q, _intnl[_qp], _dintnl);

  std::array<double, _num_rhs * _num_rhs> jac;
  dnRHSdVar(smoothed_q, _dintnl, gaE, jac);

  // use LAPACK to solve the linear system
  const int nrhs = 1;
  std::array<int, _num_rhs> ipiv;
  int info;
  const int gesv_num_rhs = _num_rhs;
  LAPACKgesv_(
      &gesv_num_rhs, &nrhs, &jac[0], &gesv_num_rhs, &ipiv[0], &_rhs[0], &gesv_num_rhs, &info);
  return info;
}

Real
TwoParameterPlasticityStressUpdate::calculateRHS(
    Real p_trial, Real q_trial, Real p, Real q, Real gaE, const f_and_derivs & smoothed_q)
{
  _rhs[0] = smoothed_q.f;
  _rhs[1] = p - p_trial + gaE * smoothed_q.dg[0];
  _rhs[2] = q - q_trial + _Eqq * gaE / _Epp * smoothed_q.dg[1];
  return _rhs[0] * _rhs[0] + _rhs[1] * _rhs[1] + _rhs[2] * _rhs[2];
}

void
TwoParameterPlasticityStressUpdate::errorHandler(const std::string & message)
{
  throw MooseException(message);
}

void
TwoParameterPlasticityStressUpdate::initialiseReturnProcess()
{
  return;
}

void
TwoParameterPlasticityStressUpdate::finalizeReturnProcess(
    const RankTwoTensor & /*rotation_increment*/)
{
  return;
}

void
TwoParameterPlasticityStressUpdate::preReturnMap(Real /*p_trial*/,
                                                 Real /*q_trial*/,
                                                 const RankTwoTensor & /*stress_trial*/,
                                                 const std::vector<Real> & /*intnl_old*/,
                                                 const std::vector<Real> & /*yf*/,
                                                 const RankFourTensor & /*Eijkl*/)
{
  return;
}

Real
TwoParameterPlasticityStressUpdate::yieldF(Real p, Real q, const std::vector<Real> & intnl) const
{
  std::vector<Real> yf(_num_yf);
  yieldFunctionValues(p, q, intnl, yf);
  std::sort(yf.begin(), yf.end());
  unsigned num = yf.size();
  while (num > 1 && yf[num - 1] < yf[num - 2] + _smoothing_tol)
  {
    yf[num - 2] = yf[num - 1] + ismoother(yf[num - 2] - yf[num - 1]);
    yf.pop_back();
    num = yf.size();
  }
  return yf.back();
}

void
TwoParameterPlasticityStressUpdate::initialiseVars(Real p_trial,
                                                   Real q_trial,
                                                   const std::vector<Real> & intnl_old,
                                                   Real & p,
                                                   Real & q,
                                                   Real & gaE,
                                                   std::vector<Real> & intnl) const
{
  p = p_trial;
  q = q_trial;
  gaE = 0.0;
  std::copy(intnl_old.begin(), intnl_old.end(), intnl.begin());
}

void
TwoParameterPlasticityStressUpdate::consistentTangentOperator(
    const RankTwoTensor & stress_trial,
    Real /*p_trial*/,
    Real /*q_trial*/,
    const RankTwoTensor & stress,
    Real /*p*/,
    Real /*q*/,
    Real gaE,
    const f_and_derivs & smoothed_q,
    const RankFourTensor & elasticity_tensor,
    bool compute_full_tangent_operator,
    RankFourTensor & cto) const
{
  if (!_fe_problem.currentlyComputingJacobian())
    return;

  cto = elasticity_tensor;
  if (!compute_full_tangent_operator)
    return;

  const RankTwoTensor dpdsig = dpdstress(stress);
  const RankTwoTensor dpdsig_trial = dpdstress(stress_trial);
  const RankTwoTensor dqdsig = dqdstress(stress);
  const RankTwoTensor dqdsig_trial = dqdstress(stress_trial);

  const RankTwoTensor s1 = elasticity_tensor * ((1.0 / _Epp) * (1.0 - _dp_dpt) * dpdsig +
                                                (1.0 / _Eqq) * (-_dq_dpt) * dqdsig);
  const RankTwoTensor s2 = elasticity_tensor * ((1.0 / _Epp) * (-_dp_dqt) * dpdsig +
                                                (1.0 / _Eqq) * (1.0 - _dq_dqt) * dqdsig);
  const RankTwoTensor t1 = elasticity_tensor * dpdsig_trial;
  const RankTwoTensor t2 = elasticity_tensor * dqdsig_trial;

  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
          cto(i, j, k, l) -= s1(i, j) * t1(k, l) + s2(i, j) * t2(k, l);

  const RankFourTensor d2pdsig2 = d2pdstress2(stress);
  const RankFourTensor d2qdsig2 = d2qdstress2(stress);

  const RankFourTensor Tijab = elasticity_tensor * (gaE / _Epp) *
                               (smoothed_q.dg[0] * d2pdsig2 + smoothed_q.dg[1] * d2qdsig2);

  RankFourTensor inv = RankFourTensor(RankFourTensor::initIdentityFour) + Tijab;
  try
  {
    inv = inv.transposeMajor().invSymm();
  }
  catch (const MooseException & e)
  {
    // Cannot form the inverse, so probably at some degenerate place in stress space.
    // Just return with the "best estimate" of the cto.
    mooseWarning("TwoParameterPlasticityStressUpdate: Cannot invert 1+T in consistent tangent "
                 "operator computation at quadpoint ",
                 _qp,
                 " of element ",
                 _current_elem->id());
    return;
  }

  cto = (cto.transposeMajor() * inv).transposeMajor();
}

void
TwoParameterPlasticityStressUpdate::setStressAfterReturn(const RankTwoTensor & stress_trial,
                                                         Real /*p_ok*/,
                                                         Real /*q_ok*/,
                                                         Real gaE,
                                                         const std::vector<Real> & /*intnl*/,
                                                         const f_and_derivs & smoothed_q,
                                                         const RankFourTensor & elasticity_tensor,
                                                         RankTwoTensor & stress) const
{
  const RankTwoTensor correction = elasticity_tensor * (smoothed_q.dg[0] * dpdstress(stress) +
                                                        smoothed_q.dg[1] * dqdstress(stress));
  stress = stress_trial - gaE / _Epp * correction;
}

void
TwoParameterPlasticityStressUpdate::setInelasticStrainIncrementAfterReturn(
    const RankTwoTensor & /*stress_trial*/,
    Real gaE,
    const f_and_derivs & smoothed_q,
    const RankFourTensor & /*elasticity_tensor*/,
    const RankTwoTensor & returned_stress,
    RankTwoTensor & inelastic_strain_increment) const
{
  inelastic_strain_increment = (gaE / _Epp) * (smoothed_q.dg[0] * dpdstress(returned_stress) +
                                               smoothed_q.dg[1] * dqdstress(returned_stress));
}
