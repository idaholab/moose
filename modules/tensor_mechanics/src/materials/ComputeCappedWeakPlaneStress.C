/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeCappedWeakPlaneStress.h"

#include "Conversion.h" // for stringify
#include "libmesh/utility.h"

template<>
InputParameters validParams<ComputeCappedWeakPlaneStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Capped weak-plane plasticity stress calculator");
  params.addRequiredParam<UserObjectName>("cohesion", "A TensorMechanicsHardening UserObject that defines hardening of the cohesion.  Physically the cohesion should not be negative.");
  params.addRequiredParam<UserObjectName>("tan_friction_angle", "A TensorMechanicsHardening UserObject that defines hardening of tan(friction angle).  Physically the friction angle should be between 0 and 90deg.");
  params.addRequiredParam<UserObjectName>("tan_dilation_angle", "A TensorMechanicsHardening UserObject that defines hardening of the tan(dilation angle).  Usually the dilation angle is not greater than the friction angle, and it is between 0 and 90deg.");
  params.addRequiredParam<UserObjectName>("tensile_strength", "A TensorMechanicsHardening UserObject that defines hardening of the weak-plane tensile strength.  In physical situations this is positive (and always must be greater than negative compressive-strength.");
  params.addRequiredParam<UserObjectName>("compressive_strength", "A TensorMechanicsHardening UserObject that defines hardening of the weak-plane compressive strength.  In physical situations this is positive.");
  params.addRangeCheckedParam<unsigned int>("max_NR_iterations", 20, "max_NR_iterations>0", "Maximum number of Newton-Raphson iterations allowed during the return-map algorithm");
  params.addRequiredRangeCheckedParam<Real>("tip_smoother", "tip_smoother>=0", "The cone vertex at shear-stress = 0 will be smoothed by the given amount.  Typical value is 0.1*cohesion");
  params.addParam<bool>("perform_finite_strain_rotations", false, "Tensors are correctly rotated in finite-strain simulations.  For optimal performance you can set this to 'false' if you are only ever using small strains");
  params.addRequiredParam<Real>("smoothing_tol", "Intersections of the yield surfaces will be smoothed by this amount (this is measured in units of stress).  Often this is 0.1*cohesion, but it is important to set this small enough so that the tensile and compressive yield do not mix together in the smoothing process, otherwise no stresses will be admissible");
  params.addRequiredParam<Real>("yield_function_tol", "The return-map process will be deemed to have converged if all yield functions are within yield_function_tol of zero.");
  MooseEnum tangent_operator("elastic nonlinear", "nonlinear");
  params.addParam<MooseEnum>("tangent_operator", tangent_operator, "Type of tangent operator to return.  'elastic': return the elasticity tensor.  'nonlinear': return the full consistent tangent operator.");
  params.addParam<bool>("perfect_guess", true, "Provide a guess to the Newton-Raphson proceedure that is the result from perfect plasticity.  With severed hardening/softening this is suboptimal.");
  params.addParam<Real>("min_step_size", 1.0E-3, "In order to help the Newton-Raphson procedure, the applied strain increment may be applied in sub-increments of size greater than this value.");
  return params;
}

ComputeCappedWeakPlaneStress::ComputeCappedWeakPlaneStress(const InputParameters & parameters) :
    ComputeStressBase(parameters),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _tan_phi(getUserObject<TensorMechanicsHardeningModel>("tan_friction_angle")),
    _tan_psi(getUserObject<TensorMechanicsHardeningModel>("tan_dilation_angle")),
    _tstrength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _cstrength(getUserObject<TensorMechanicsHardeningModel>("compressive_strength")),
    _small_smoother2(Utility::pow<2>(getParam<Real>("tip_smoother"))),
    _max_nr_its(getParam<unsigned>("max_NR_iterations")),
    _perform_finite_strain_rotations(getParam<bool>("perform_finite_strain_rotations")),
    _smoothing_tol(getParam<Real>("smoothing_tol")),
    _f_tol(getParam<Real>("yield_function_tol")),
    _f_tol2(Utility::pow<2>(getParam<Real>("yield_function_tol"))),
    _tangent_operator_type((TangentOperatorEnum)(int)getParam<MooseEnum>("tangent_operator")),
    _perfect_guess(getParam<bool>("perfect_guess")),
    _stress_return_type(StressReturnType::nothing_special),
    _min_step_size(getParam<Real>("min_step_size")),
    _step_one(declareRestartableData<bool>("step_one", true)),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
    _intnl(declareProperty<std::vector<Real> >("plastic_internal_parameter")),
    _intnl_old(declarePropertyOld<std::vector<Real> >("plastic_internal_parameter")),
    _yf(declareProperty<std::vector<Real> >("plastic_yield_function")),
    _iter(declareProperty<Real>("plastic_NR_iterations")), // this is really an unsigned int, but for visualisation i convert it to Real
    _linesearch_needed(declareProperty<Real>("plastic_linesearch_needed")), // this is really a boolean, but for visualisation i convert it to Real

    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),

    _stress_old(declarePropertyOld<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "elastic_strain")),

    _dgaE_dpt(0.0),
    _dp_dpt(0.0),
    _dq_dpt(0.0),
    _dgaE_dqt(0.0),
    _dp_dqt(0.0),
    _dq_dqt(0.0),
    //DELETE?_rhs(_num_rhs),
    _all_q(_num_yf)
{
  // With arbitary UserObjects, it is impossible to check everything,
  // but this will catch the common errors
  if (_tan_phi.value(0) < 0 || _tan_psi.value(0) < 0)
    mooseError("ComputeCappedWeakPlaneStress: Weak-plane friction and dilation angles must lie in [0, Pi/2]");
  if (_tan_phi.value(0) < _tan_psi.value(0))
    mooseError("ComputeCappedWeakPlaneStress: Weak-plane friction angle must not be less than dilation angle");
  if (_cohesion.value(0) < 0)
    mooseError("ComputeCappedWeakPlaneStress: Weak-plane cohesion must not be negative");
  if (_tstrength.value(0) < - _cstrength.value(0))
    mooseError("ComputeCappedWeakPlaneStress: Weak plane tensile strength must not be less than negative-compressive-strength");

  for (unsigned i = 0; i < _num_yf; ++i)
    _all_q[i] = f_and_derivs(_num_pq, _num_intnl);
}

void
ComputeCappedWeakPlaneStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();

  _plastic_strain[_qp].zero();
  _intnl[_qp].assign(_num_intnl, 0);
  _yf[_qp].assign(_num_yf, 0);
  _iter[_qp] = 0.0;
  _linesearch_needed[_qp] = 0.0;
}

void
ComputeCappedWeakPlaneStress::computeQpStress()
{
  if (_t_step >= 2)
    _step_one = false;

  returnMap();

  //Update measures of strain
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp] - (_plastic_strain[_qp] - _plastic_strain_old[_qp]);

  //Rotate the tensors to the current configuration
  if (_perform_finite_strain_rotations)
  {
    _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();
    _elastic_strain[_qp] = _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();
    _plastic_strain[_qp] = _rotation_increment[_qp] * _plastic_strain[_qp] * _rotation_increment[_qp].transpose();
  }
}

void
ComputeCappedWeakPlaneStress::returnMap()
{
  // initially assume an elastic deformation
  _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp];
  for (unsigned i = 0; i < _num_intnl; ++i)
    _intnl[_qp][i] = _intnl_old[_qp][i];
  _iter[_qp] = 0.0;
  _linesearch_needed[_qp] = 0.0;

  _stress_return_type = StressReturnType::nothing_special;

  Real p_trial = _stress[_qp](2, 2);
  Real q_trial = std::sqrt(Utility::pow<2>(_stress[_qp](2, 0)) + Utility::pow<2>(_stress[_qp](2, 1)));
  unsmoothedYieldFunctions(p_trial, q_trial, _intnl[_qp], _yf[_qp]);

  /* Need to consider smoothing_tol, not just yf<=0, because
   * some yield functions might mix.  However, if some yield
   * functions are -smoothing_tol <= yf < 0, then the smoothed
   * yield function must be computed and checked vs f_tol
   */
  if ((_yf[_qp][0] < -_smoothing_tol &&
      _yf[_qp][1] < -_smoothing_tol &&
      _yf[_qp][2] < -_smoothing_tol) ||
      yieldF(p_trial, q_trial, _intnl[_qp]) <= _f_tol)
  {
    // elastic case
    _plastic_strain[_qp] = _plastic_strain_old[_qp];
    if (_fe_problem.currentlyComputingJacobian())
      _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
    return;
  }


  /* The trial stress (_stress[_qp]) must be inadmissible
   * so we need to return to the yield surface.  The following
   * equations must be satisfied.
   * 0 = f(p, q, intnl)                    = rhs[0]
   * 0 = p - p^trial + Ezzzz * ga * dg/dp  = rhs[1]
   * 0 = q - q^trial + Ezxzx * ga * dg/dq  = rhs[2]
   * 0 = intnl[0] - intnl_old[0] - (q^trial - q)/Ezxzx
   * 0 = intnl[1] - intnl_old[1] - (p^trial - p)/Ezzzz + (q^trial - q) * tanpsi / Ezxzx
   * With unknowns p, q, ga, intnl[0] and intnl[1]
   * I find it convenient to solve the first three equations for p, q and ga*Ezzzz,
   * while substituting the last two equations into these during the solve process
   * (tanpsi depends only on intnl[0], and hence on q)
   */

  const Real in_trial20 = _stress[_qp](2, 0);
  const Real in_trial21 = _stress[_qp](2, 1);
  const Real in_q_trial = q_trial;

  // values of p, q  and intnl we know to be admissible
  Real p_ok = _stress_old[_qp](2, 2);
  Real q_ok = std::sqrt(Utility::pow<2>(_stress[_qp](2, 0)) + Utility::pow<2>(_stress[_qp](2, 1)));
  // note that _intnl[_qp] is the "running" intnl variable that gets updated every NR iteration
  std::array<Real, 2> intnl_ok = {{ _intnl_old[_qp][0], _intnl_old[_qp][1] }};
  if (isParamValid("initial_stress") && _step_one)
  {
    // the initial stress might be inadmissible
    p_ok = 0.0;
    q_ok = 0.0;
  }

  _dgaE_dpt = 0.0;
  _dp_dpt = 0.0;
  _dq_dpt = 0.0;
  _dgaE_dqt = 0.0;
  _dp_dqt = 0.0;
  _dq_dqt = 0.0;

  // Return-map problem: must apply the following changes in p and q, and find the returned p and q.
  const Real del_p = p_trial - p_ok;
  const Real del_q = q_trial - q_ok;

  // If it's obvious, then simplify the return-type
  if (_yf[_qp][1] >= 0)
    _stress_return_type = StressReturnType::no_compression;
  else if (_yf[_qp][2] >= 0)
    _stress_return_type = StressReturnType::no_tension;

  Real step_taken = 0.0; // amount of (del_p, del_q) that we've applied and the return-map problem has succeeded
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
    p_trial = p_ok + step_size * del_p;
    q_trial = q_ok + step_size * del_q;

    // initialise (p, q, gaE)
    Real p = p_trial;
    Real q = q_trial;
    Real gaE = 0.0;

    // flags indicating failure of newton-raphson and line-search
    int nr_failure = 0;
    int ls_failure = 0;

    // NR iterations taken in this substep
    Real step_iter = 0.0;

    // The residual-squared for the line-search
    Real res2 = 0.0;

    if (step_size < 1.0 && yieldF(p_trial, q_trial, _intnl[_qp]) <= _f_tol)
      // This is an elastic step
      // The "step_size < 1.0" in above condition is for efficiency: we definitely
      // know that this is a plastic step if step_size = 1.0
      smoothed_q_calculated = false;
    else
    {
      // this is a plastic step

      // initialise p, q and gaE using a good guess based on the non-smoothed situation
      initialiseVars(p_trial, q_trial, p, q, gaE);
      smoothed_q = smoothAllQuantities(p, q, _intnl[_qp]);
      smoothed_q_calculated = true;
      res2 = calculateRHS(p_trial, q_trial, p, q, gaE, smoothed_q);

      // Perform a Newton-Raphson with linesearch to get p, q, gaE, and also smoothed_q
      while (res2 > _f_tol2 && step_iter < (float) _max_nr_its && nr_failure == 0 && ls_failure == 0)
      {
        // solve the linear system and store the answer (the "updates") in _rhs
        nr_failure = nrStep(smoothed_q, q_trial, q, gaE);
        if (nr_failure != 0)
          break;

        // apply (parts of) the updates, re-calculate smoothed_q, and res2
        ls_failure = lineSearch(res2, gaE, p, q, q_trial, p_trial, smoothed_q, intnl_ok);
        step_iter++;
      }
    }

    if (res2 <= _f_tol2 && step_iter < (float) _max_nr_its && nr_failure == 0 && ls_failure == 0 && gaE >= 0.0)
    {
      // this Newton-Raphson worked fine, or this was an elastic step
      p_ok = p;
      q_ok = q;
      step_taken += step_size;
      gaE_total += gaE;
      const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
      const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);
      intnl_ok[0] = _intnl[_qp][0] = intnl_ok[0] + (q_trial - q_ok) / Ezxzx;
      const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
      intnl_ok[1] = _intnl[_qp][1] = intnl_ok[1] + (p_trial - p_ok) / Ezzzz - (q_trial - q_ok) * tanpsi / Ezxzx;
      // calculate dp/dp_trial, dp/dq_trial, etc, for Jacobian
      dVardTrial(!smoothed_q_calculated, q_ok, q_trial, gaE, smoothed_q, step_size);
      if (step_iter > _iter[_qp])
        _iter[_qp] = step_iter;
      step_size *= 1.1;
    }
    else
    {
      // Newton-Raphson failed
      step_size *= 0.5;
    }
  }


  if (step_size < _min_step_size)
    throw MooseException("ComputeCappedWeakPlaneStress: Minimum step-size violated");

  // success!
  _stress_return_type = StressReturnType::nothing_special;
  unsmoothedYieldFunctions(p_ok, q_ok, _intnl[_qp], _yf[_qp]);

  if (!smoothed_q_calculated)
    smoothed_q = smoothAllQuantities(p_ok, q_ok, _intnl[_qp]);

  _stress[_qp](2, 2) = p_ok;
  // stress_xx and stress_yy are sitting at their trial-stress values
  // so need to bring them back via Poisson's ratio
  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  _stress[_qp](0, 0) -= _elasticity_tensor[_qp](2, 2, 0, 0) * gaE_total / Ezzzz * smoothed_q.dg[0];
  _stress[_qp](1, 1) -= _elasticity_tensor[_qp](2, 2, 1, 1) * gaE_total / Ezzzz * smoothed_q.dg[0];
  if (in_q_trial == 0.0)
    _stress[_qp](2, 0) = _stress[_qp](2, 1) = _stress[_qp](0, 2) = _stress[_qp](1, 2) = 0.0;
  else
  {
    _stress[_qp](2, 0) = _stress[_qp](0, 2) = in_trial20 * q_ok / in_q_trial;
    _stress[_qp](2, 1) = _stress[_qp](1, 2) = in_trial21 * q_ok / in_q_trial;
  }

  _plastic_strain[_qp] = _plastic_strain_old[_qp] + _strain_increment[_qp] + _elasticity_tensor[_qp].invSymm() * (_stress_old[_qp] - _stress[_qp]);

  consistentTangentOperator(q_ok, in_q_trial, in_trial20, in_trial21, gaE_total, smoothed_q);
}

void
ComputeCappedWeakPlaneStress::dVardTrial(bool elastic_only, Real q, Real q_trial, Real gaE, const f_and_derivs & smoothed_q, Real step_size)
{
  if (!_fe_problem.currentlyComputingJacobian())
    return;

  if (_tangent_operator_type == TangentOperatorEnum::elastic)
    return;

  if (elastic_only)
  {
    // no change to gaE, and dp_dqt and dq_dpt are unchanged from previous step
    _dp_dpt = step_size + _dp_dpt;
    _dq_dqt = step_size + _dq_dqt;
    return;
  }

  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);
  const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
  const Real dintnl0_dq = -1.0 / Ezxzx;
  const Real dintnl0_dqt = 1.0 / Ezxzx;
  const Real dintnl1_dp = -1.0 / Ezzzz;
  const Real dintnl1_dpt = 1.0 / Ezzzz;
  const Real dintnl1_dq = tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dq / Ezxzx;
  const Real dintnl1_dqt = - tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dqt / Ezxzx;

  // _rhs is defined above, the following are changes in rhs wrt the trial p and q values
  std::array<Real, _num_pq * _num_rhs> rhs_cto;
  // change in p_trial
  rhs_cto[0] = - smoothed_q.df_di[1] * dintnl1_dpt;
  rhs_cto[1] = 1.0 - gaE * smoothed_q.d2g_di[0][1] * dintnl1_dpt;
  rhs_cto[2] = - Ezxzx * gaE / Ezzzz * smoothed_q.d2g_di[1][1] * dintnl1_dpt;
  // change in q_trial
  rhs_cto[3] = - smoothed_q.df_di[0] * dintnl0_dqt - smoothed_q.df_di[1] * dintnl1_dqt;
  rhs_cto[4] = - gaE * (smoothed_q.d2g_di[0][0] * dintnl0_dqt + smoothed_q.d2g_di[0][1] * dintnl1_dqt);
  rhs_cto[5] = 1.0 - Ezxzx * gaE / Ezzzz * (smoothed_q.d2g_di[1][0] * dintnl0_dqt + smoothed_q.d2g_di[1][1] * dintnl1_dqt);

  // jac = d(rhs)/d(var), where var[0] = gaE, var[1] = p, var[2] = q.
  std::array<double, _num_rhs * _num_rhs> jac;
  jac[0] = 0;
  jac[1] = smoothed_q.dg[0];
  jac[2] = Ezxzx * smoothed_q.dg[1] / Ezzzz;
  jac[3] = (smoothed_q.df[0] + smoothed_q.df_di[1] * dintnl1_dp);
  jac[4] = 1.0 + gaE * (smoothed_q.d2g[0][0] + smoothed_q.d2g_di[0][1] * dintnl1_dp);
  jac[5] = Ezxzx * gaE / Ezzzz * (smoothed_q.d2g[1][0] + smoothed_q.d2g_di[1][1] * dintnl1_dp);
  jac[6] = (smoothed_q.df[1] + smoothed_q.df_di[0] * dintnl0_dq + smoothed_q.df_di[1] * dintnl1_dq);
  jac[7] = gaE * (smoothed_q.d2g[0][1] + smoothed_q.d2g_di[0][0] * dintnl0_dq + smoothed_q.d2g_di[0][1] * dintnl1_dq);
  jac[8] = 1.0 + Ezxzx * gaE / Ezzzz * (smoothed_q.d2g[1][1] + smoothed_q.d2g_di[1][0] * dintnl0_dq + smoothed_q.d2g_di[1][1] * dintnl1_dq);

  std::array<int, _num_rhs> ipiv;
  int info;
  const int gesv_num_rhs = _num_rhs;
  const int gesv_num_pq = _num_pq;
  LAPACKgesv_(&gesv_num_rhs, &gesv_num_pq, &jac[0], &gesv_num_rhs, &ipiv[0], &rhs_cto[0], &gesv_num_rhs, &info);
  if (info != 0)
    throw MooseException("ComputeCappedWeakPlaneStress: PETSC LAPACK gsev routine returned with error code " + Moose::stringify(info));

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

void
ComputeCappedWeakPlaneStress::consistentTangentOperator(Real q, Real q_trial, Real trial20, Real trial21, Real gaE, const f_and_derivs & smoothed_q)
{
  if (!_fe_problem.currentlyComputingJacobian())
    return;

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  if (_tangent_operator_type == TangentOperatorEnum::elastic)
    return;

  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);
  const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
  const Real dintnl0_dq = -1.0 / Ezxzx;
  const Real dintnl0_dqt = 1.0 / Ezxzx;
  const Real dintnl1_dp = -1.0 / Ezzzz;
  const Real dintnl1_dpt = 1.0 / Ezzzz;
  const Real dintnl1_dq = tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dq / Ezxzx;
  const Real dintnl1_dqt = - tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dqt / Ezxzx;

  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
  {
    const Real dpt_depii = _elasticity_tensor[_qp](2, 2, i, i);
    _Jacobian_mult[_qp](2, 2, i, i) = _dp_dpt * dpt_depii;
    const Real poisson_effect = _elasticity_tensor[_qp](2, 2, 0, 0) / Ezzzz * (_dgaE_dpt * smoothed_q.dg[0] + gaE * smoothed_q.d2g[0][0] * _dp_dpt + gaE * smoothed_q.d2g[0][1] * _dq_dpt + gaE * smoothed_q.d2g_di[0][0] * (dintnl0_dq * _dq_dpt) + gaE * smoothed_q.d2g_di[0][1] * (dintnl1_dpt + dintnl1_dp * _dp_dpt + dintnl1_dq * _dq_dpt)) * dpt_depii;
    _Jacobian_mult[_qp](0, 0, i, i) -= poisson_effect;
    _Jacobian_mult[_qp](1, 1, i, i) -= poisson_effect;
    if (q_trial > 0.0)
    {
      _Jacobian_mult[_qp](2, 0, i, i) = _Jacobian_mult[_qp](0, 2, i, i) = trial20 / q_trial * _dq_dpt * dpt_depii;
      _Jacobian_mult[_qp](2, 1, i, i) = _Jacobian_mult[_qp](1, 2, i, i) = trial21 / q_trial * _dq_dpt * dpt_depii;
    }
  }

  const Real poisson_effect = - _elasticity_tensor[_qp](2, 2, 0, 0) / Ezzzz * (_dgaE_dqt * smoothed_q.dg[0] + gaE * smoothed_q.d2g[0][0] * _dp_dqt + gaE * smoothed_q.d2g[0][1] * _dq_dqt + gaE * smoothed_q.d2g_di[0][0] * (dintnl0_dqt + dintnl0_dq * _dq_dqt) + gaE * smoothed_q.d2g_di[0][1] * (dintnl1_dqt + dintnl1_dp * _dp_dqt + dintnl1_dq * _dq_dqt));

  const Real dqt_dep20 = (q_trial == 0.0 ? 1.0 : trial20 / q_trial) * _elasticity_tensor[_qp](2, 0, 2, 0);
  _Jacobian_mult[_qp](2, 2, 2, 0) = _Jacobian_mult[_qp](2, 2, 0, 2) = _dp_dqt * dqt_dep20;
  _Jacobian_mult[_qp](0, 0, 2, 0) = _Jacobian_mult[_qp](0, 0, 0, 2) = _Jacobian_mult[_qp](1, 1, 2, 0) = _Jacobian_mult[_qp](1, 1, 0, 2) =  poisson_effect* dqt_dep20;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    _Jacobian_mult[_qp](0, 2, 0, 2) = _Jacobian_mult[_qp](2, 0, 0, 2) = _Jacobian_mult[_qp](0, 2, 2, 0) = _Jacobian_mult[_qp](2, 0, 2, 0) = _elasticity_tensor[_qp](2, 0, 2, 0) * q / q_trial + trial20 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
    _Jacobian_mult[_qp](1, 2, 0, 2) = _Jacobian_mult[_qp](2, 1, 0, 2) = _Jacobian_mult[_qp](1, 2, 2, 0) = _Jacobian_mult[_qp](2, 1, 2, 0) = trial21 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
  }

  const Real dqt_dep21 = (q_trial == 0.0 ? 1.0 : trial21 / q_trial) * _elasticity_tensor[_qp](2, 1, 2, 1);
  _Jacobian_mult[_qp](2, 2, 2, 1) = _Jacobian_mult[_qp](2, 2, 1, 2) = _dp_dqt * dqt_dep21;
  _Jacobian_mult[_qp](0, 0, 2, 1) = _Jacobian_mult[_qp](0, 0, 1, 2) = _Jacobian_mult[_qp](1, 1, 2, 1) = _Jacobian_mult[_qp](1, 1, 1, 2) = poisson_effect * dqt_dep21;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    _Jacobian_mult[_qp](0, 2, 1, 2) = _Jacobian_mult[_qp](2, 0, 1, 2) = _Jacobian_mult[_qp](0, 2, 2, 1) = _Jacobian_mult[_qp](2, 0, 2, 1) = trial20 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
    _Jacobian_mult[_qp](1, 2, 1, 2) = _Jacobian_mult[_qp](2, 1, 1, 2) = _Jacobian_mult[_qp](1, 2, 2, 1) = _Jacobian_mult[_qp](2, 1, 2, 1) = _elasticity_tensor[_qp](2, 1, 2, 1) * q / q_trial + trial21 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
  }
}

void
ComputeCappedWeakPlaneStress::unsmoothedYieldFunctions(Real p, Real q, const std::vector<Real> & intnl, std::vector<Real> & yf) const
{
  mooseAssert(intnl.size() == _num_intnl, "ComputeCappedWeakPlaneStress: yieldFunctions called with intnl size " << intnl.size());;
  mooseAssert(yf.size() == _num_yf, "ComputeCappedWeakPlaneStress: yieldFunctions called with yf size " << yf.size());
  yf[0] = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * _tan_phi.value(intnl[0]) - _cohesion.value(intnl[0]);
  if (_stress_return_type == StressReturnType::no_tension)
    yf[1] = std::numeric_limits<Real>::lowest();
  else
    yf[1] = p - _tstrength.value(intnl[1]);
  if (_stress_return_type == StressReturnType::no_compression)
    yf[2] = std::numeric_limits<Real>::lowest();
  else
    yf[2] = - p - _cstrength.value(intnl[1]);
}

void
ComputeCappedWeakPlaneStress::unsmoothedYieldFunctions(Real p, Real q, const std::vector<Real> & intnl)
{
  _all_q[0].f = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * _tan_phi.value(intnl[0]) - _cohesion.value(intnl[0]);
  if (_stress_return_type == StressReturnType::no_tension)
    _all_q[1].f = std::numeric_limits<Real>::lowest();
  else
    _all_q[1].f = p - _tstrength.value(intnl[1]);
  if (_stress_return_type == StressReturnType::no_compression)
    _all_q[2].f = std::numeric_limits<Real>::lowest();
  else
    _all_q[2].f = - p - _cstrength.value(intnl[1]);
}

void
ComputeCappedWeakPlaneStress::dyieldFunctions(Real /*p*/, Real q, const std::vector<Real> & intnl)
{
  // derivatives wrt p
  _all_q[0].df[0] = _tan_phi.value(intnl[0]);
  _all_q[1].df[0] = 1.0;
  _all_q[2].df[0] = - 1.0;

  // derivatives wrt q
  if (_small_smoother2 == 0.0)
    _all_q[0].df[1] = 1.0;
  else
    _all_q[0].df[1] = q / std::sqrt(Utility::pow<2>(q) + _small_smoother2);
  _all_q[1].df[1] = 0.0;
  _all_q[2].df[1] = 0.0;
}

void
ComputeCappedWeakPlaneStress::dyieldFunctions_di(Real p, Real /*q*/, const std::vector<Real> & intnl)
{
  // derivatives wrt intnl[0] (shear plastic strain)
  _all_q[0].df_di[0] = p * _tan_phi.derivative(intnl[0]) - _cohesion.derivative(intnl[0]);
  _all_q[1].df_di[0] = 0.0;
  _all_q[2].df_di[0] = 0.0;
  // derivatives wrt intnl[q] (tensile plastic strain)
  _all_q[0].df_di[1] = 0.0;
  _all_q[1].df_di[1] = - _tstrength.derivative(intnl[1]);
  _all_q[2].df_di[1] = - _cstrength.derivative(intnl[1]);
}

void
ComputeCappedWeakPlaneStress::dflowPotential(Real /*p*/, Real q, const std::vector<Real> & intnl)
{
  // derivatives wrt p
  _all_q[0].dg[0] = _tan_psi.value(intnl[0]);
  _all_q[1].dg[0] = 1.0;
  _all_q[2].dg[0] = - 1.0;
  // derivatives wrt q
  if (_small_smoother2 == 0.0)
    _all_q[0].dg[1] = 1.0;
  else
    _all_q[0].dg[1] = q / std::sqrt(Utility::pow<2>(q) + _small_smoother2);
  _all_q[1].dg[1] = 0.0;
  _all_q[2].dg[1] = 0.0;
}

void
ComputeCappedWeakPlaneStress::d2flowPotential_di(Real /*p*/, Real /*q*/, const std::vector<Real> & intnl)
{
  // d(dg/dp)/dintnl[0]
  _all_q[0].d2g_di[0][0] = _tan_psi.derivative(intnl[0]);
  _all_q[1].d2g_di[0][0] = 0.0;
  _all_q[2].d2g_di[0][0] = 0.0;
  // d(dg/dp)/dintnl[1]
  _all_q[0].d2g_di[0][1] = 0.0;
  _all_q[1].d2g_di[0][1] = 0.0;
  _all_q[2].d2g_di[0][1] = 0.0;
  // d(dg/dq)/dintnl[0]
  _all_q[0].d2g_di[1][0] = 0.0;
  _all_q[1].d2g_di[1][0] = 0.0;
  _all_q[2].d2g_di[1][0] = 0.0;
  // d(dg/dq)/dintnl[1]
  _all_q[0].d2g_di[1][1] = 0.0;
  _all_q[1].d2g_di[1][1] = 0.0;
  _all_q[2].d2g_di[1][1] = 0.0;
}

void
ComputeCappedWeakPlaneStress::d2flowPotential(Real /*p*/, Real q, const std::vector<Real> & /*intnl*/)
{
  // d(dg/dp)/dp
  _all_q[0].d2g[0][0] = 0.0;
  _all_q[1].d2g[0][0] = 0.0;
  _all_q[2].d2g[0][0] = 0.0;
  // d(dg/dp)/dq
  _all_q[0].d2g[0][1] = 0.0;
  _all_q[1].d2g[0][1] = 0.0;
  _all_q[2].d2g[0][1] = 0.0;
  // d(dg/dq)/dp
  _all_q[0].d2g[1][0] = 0.0;
  _all_q[1].d2g[1][0] = 0.0;
  _all_q[2].d2g[1][0] = 0.0;
  // d(dg/dq)/dq
  if (_small_smoother2 == 0.0)
    _all_q[0].d2g[1][1] = 0.0;
  else
    _all_q[0].d2g[1][1] = _small_smoother2 / std::pow(Utility::pow<2>(q) + _small_smoother2, 1.5);
  _all_q[1].d2g[1][1] = 0.0;
  _all_q[2].d2g[1][1] = 0.0;
}

Real
ComputeCappedWeakPlaneStress::yieldF(Real p, Real q, const std::vector<Real> & intnl) const
{
  std::vector<Real> yf(_num_yf);
  unsmoothedYieldFunctions(p, q, intnl, yf);
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

ComputeCappedWeakPlaneStress::f_and_derivs
ComputeCappedWeakPlaneStress::smoothAllQuantities(Real p, Real q, const std::vector<Real> & intnl)
{
  for (unsigned i = _all_q.size(); i < _num_yf; ++i)
    _all_q.push_back(f_and_derivs(2, 2));

  unsmoothedYieldFunctions(p, q, intnl);
  dyieldFunctions(p, q, intnl);
  dyieldFunctions_di(p, q, intnl);
  dflowPotential(p, q, intnl);
  d2flowPotential(p, q, intnl);
  d2flowPotential_di(p, q, intnl);

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
   * d(second-biggest-g) = d(biggest-g) + smoother(second-biggest-f - biggest-f)*(d(second-biggest-g) - d(biggest-g))
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
        _all_q[num - 2].d2g[i][j] = _all_q[num - 1].d2g[i][j] + dsm * (_all_q[num - 2].df[j] - _all_q[num - 1].df[j]) * (_all_q[num - 2].dg[i] - _all_q[num - 1].dg[i]) + sm * (_all_q[num - 2].d2g[i][j] - _all_q[num - 1].d2g[i][j]);
      for (unsigned j = 0; j < _num_intnl; ++j)
        _all_q[num - 2].d2g_di[i][j] = _all_q[num - 1].d2g_di[i][j] + dsm * (_all_q[num - 2].df_di[j] - _all_q[num - 1].df_di[j]) * (_all_q[num - 2].dg[i] - _all_q[num - 1].dg[i]) + sm * (_all_q[num - 2].d2g_di[i][j] - _all_q[num - 1].d2g_di[i][j]);
    }
    for (unsigned i = 0; i < _num_pq; ++i)
    {
      _all_q[num - 2].dg[i] = _all_q[num - 1].dg[i] + sm * (_all_q[num - 2].dg[i] - _all_q[num - 1].dg[i]);
      _all_q[num - 2].df[i] = _all_q[num - 1].df[i] + sm * (_all_q[num - 2].df[i] - _all_q[num - 1].df[i]);
    }
    for (unsigned i = 0; i < _num_intnl; ++i)
      _all_q[num - 2].df_di[i] = _all_q[num - 1].df_di[i] + sm * (_all_q[num - 2].df_di[i] - _all_q[num - 1].df_di[i]);
    _all_q[num - 2].f = _all_q[num - 1].f + ism;
    _all_q.pop_back();
    num = _all_q.size();
  }
  return _all_q.back();
}


Real
ComputeCappedWeakPlaneStress::ismoother(Real f_diff) const
{
  mooseAssert(f_diff <= 0.0, "ComputeCappedWeakPlaneStress: ismoother called with positive argument");
  if (f_diff <= -_smoothing_tol)
    return 0.0;
  return 0.5 * (f_diff + _smoothing_tol) - _smoothing_tol / M_PI * std::cos(0.5 * M_PI * f_diff / _smoothing_tol);
}


Real
ComputeCappedWeakPlaneStress::smoother(Real f_diff) const
{
  if (f_diff >= _smoothing_tol)
    return 1.0;
  else if (f_diff <= -_smoothing_tol)
    return 0.0;
  return 0.5 * (1.0 + std::sin(f_diff * M_PI * 0.5 / _smoothing_tol));
}

Real
ComputeCappedWeakPlaneStress::dsmoother(Real f_diff) const
{
  if (f_diff >= _smoothing_tol)
    return 0.0;
  else if (f_diff <= -_smoothing_tol)
    return 0.0;
  return 0.25 * M_PI / _smoothing_tol * std::cos(f_diff * M_PI * 0.5 / _smoothing_tol);
}


int
ComputeCappedWeakPlaneStress::lineSearch(Real & res2, Real & gaE, Real & p, Real & q, Real q_trial, Real p_trial, f_and_derivs & smoothed_q, const std::array<Real, 2> intnl_ok)
{
  const Real res2_old = res2;
  const Real gaE_old = gaE;
  const Real p_old = p;
  const Real q_old = q;
  const Real de_gaE = _rhs[0];
  const Real de_p = _rhs[1];
  const Real de_q = _rhs[2];


  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);

  Real lam = 1.0; // line-search parameter
  const Real lam_min = 1E-10; // minimum value of lam allowed
  const Real slope = -2.0 * res2_old; // "Numerical Recipes" uses -b*A*x, in order to check for roundoff, but i hope the nrStep would warn if there were problems
  Real tmp_lam; // cached value of lam used in quadratic & cubic line search
  Real f2 = res2_old; // cached value of f = residual2 used in the cubic in the line search
  Real lam2 = lam; // cached value of lam used in the cubic in the line search

  while (true)
  {
    // update variables using the current line-search parameter
    gaE = gaE_old + lam * de_gaE;
    p = p_old + lam * de_p;
    q = q_old + lam * de_q;

    //Moose::out << "  lam, p, q = " << lam << " " << p << " " << q << "\n";

    // and internal parameters
    _intnl[_qp][0] = intnl_ok[0] + (q_trial - q) / Ezxzx;
    const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
    _intnl[_qp][1] = intnl_ok[1] + (p_trial - p) / Ezzzz - (q_trial - q) * tanpsi / Ezxzx;

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
      tmp_lam = - 0.5 * slope / (res2 - res2_old - slope);
    }
    else
    {
      //model as a cubic
      const Real rhs1 = res2 - res2_old - lam * slope;
      const Real rhs2 = f2 - res2_old - lam2 * slope;
      const Real a = (rhs1 / Utility::pow<2>(lam) - rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
      const Real b = (-lam2 * rhs1 / Utility::pow<2>(lam) + lam * rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
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


int
ComputeCappedWeakPlaneStress::nrStep(const f_and_derivs & smoothed_q, Real q_trial, Real q, Real gaE)
{
  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);

  const Real dintnl0_dq = -1.0 / Ezxzx;
  const Real dintnl1_dp = -1.0 / Ezzzz;
  const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
  const Real dintnl1_dq = tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dq / Ezxzx;

  std::array<double, _num_rhs * _num_rhs> jac;

  // LAPACK gsev stores the matrix in the following way:
  // d(-yieldF)/d(gaE)
  jac[0] = 0;
  // d(-rhs[1])/d(gaE)
  jac[1] = - smoothed_q.dg[0];
  // d(-rhs[2])/d(gaE))
  jac[2] = - Ezxzx * smoothed_q.dg[1] / Ezzzz;
  // d(-yieldF)/d(p)
  jac[3] = - (smoothed_q.df[0] + smoothed_q.df_di[1] * dintnl1_dp);
  // d(-rhs[1])/d(p)
  jac[4] = - 1.0 - gaE * (smoothed_q.d2g[0][0] + smoothed_q.d2g_di[0][1] * dintnl1_dp);
  // d(-rhs[2])/d(p)
  jac[5] = - Ezxzx * gaE / Ezzzz * (smoothed_q.d2g[1][0] + smoothed_q.d2g_di[1][1] * dintnl1_dp);
  // d(-yieldF)/d(q)
  jac[6] = - (smoothed_q.df[1] + smoothed_q.df_di[0] * dintnl0_dq + smoothed_q.df_di[1] * dintnl1_dq);
  // d(-rhs[1])/d(q)
  jac[7] = - gaE * (smoothed_q.d2g[0][1] + smoothed_q.d2g_di[0][0] * dintnl0_dq + smoothed_q.d2g_di[0][1] * dintnl1_dq);
  // d(-rhs[2])/d(q)
  jac[8] = - 1.0 - Ezxzx * gaE / Ezzzz * (smoothed_q.d2g[1][1] + smoothed_q.d2g_di[1][0] * dintnl0_dq + smoothed_q.d2g_di[1][1] * dintnl1_dq);

  // use LAPACK to solve the linear system
  const int nrhs = 1;
  std::array<int, _num_rhs> ipiv;
  int info;
  const int gesv_num_rhs = _num_rhs;
  LAPACKgesv_(&gesv_num_rhs, &nrhs, &jac[0], &gesv_num_rhs, &ipiv[0], &_rhs[0], &gesv_num_rhs, &info);
  return info;
}

void
ComputeCappedWeakPlaneStress::initialiseVars(Real p_trial, Real q_trial, Real & p, Real & q, Real & gaE)
{
  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);
  const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);

  if (!_perfect_guess)
  {
    p = p_trial;
    q = q_trial;
    gaE = 0.0;
  }
  else
  {
    const Real coh = _cohesion.value(_intnl[_qp][0]);
    const Real tanphi = _tan_phi.value(_intnl[_qp][0]);
    const Real tens = _tstrength.value(_intnl[_qp][1]);
    const Real comp = _cstrength.value(_intnl[_qp][1]);
    const Real q_at_T = coh - tens * tanphi;
    const Real q_at_C = coh + comp * tanphi;

    if ((p_trial >= tens) && (q_trial <= q_at_T))
    {
      // pure tensile failure
      p = tens;
      q = q_trial;
      gaE = p_trial - p;
    }
    else if ((p_trial <= - comp) && (q_trial <= q_at_C))
    {
      // pure compressive failure
      p = - comp;
      q = q_trial;
      gaE = p - p_trial;
    }
    else
    {
      // shear failure or a mixture
      const Real ga = (q_trial + p_trial * tanphi - coh) / (Ezxzx + Ezzzz * tanphi * tanpsi);
      q = q_trial - Ezxzx * ga;
      if (q <= q_at_T)
      {
        // mixture of tensile and shear
        gaE = ga * Ezzzz * (q_trial - q) / (q_trial - q_at_T);
        q = q_at_T;
        p = tens;
      }
      else if (q >= q_at_C)
      {
        // mixture of compression and shear
        gaE = ga * Ezzzz * (q_trial - q) / (q_trial - q_at_C);
        q = q_at_C;
        p = - comp;
      }
      else
      {
        // pure shear
        p = p_trial - Ezzzz * ga * tanpsi;
        gaE = ga * Ezzzz;
      }
    }
  }

  _intnl[_qp][0] = _intnl_old[_qp][0] + (q_trial - q) / Ezxzx;
  _intnl[_qp][1] = _intnl_old[_qp][1] + (p_trial - p) / Ezzzz - (q_trial - q) * tanpsi / Ezxzx;
}

Real
ComputeCappedWeakPlaneStress::calculateRHS(Real p_trial, Real q_trial, Real p, Real q, Real gaE, const f_and_derivs & smoothed_q)
{
  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);

  _rhs[0] = smoothed_q.f;
  _rhs[1] = p - p_trial + gaE * smoothed_q.dg[0];
  _rhs[2] = q - q_trial + Ezxzx * gaE / Ezzzz * smoothed_q.dg[1];
  return _rhs[0] * _rhs[0] + _rhs[1] * _rhs[1] + _rhs[2] * _rhs[2];
}
