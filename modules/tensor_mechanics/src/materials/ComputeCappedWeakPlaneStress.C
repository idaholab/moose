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
  params.addRequiredParam<UserObjectName>("tensile_strength", "A TensorMechanicsHardening UserObject that defines hardening of the weak-plane tensile strength");
  params.addRequiredParam<UserObjectName>("compressive_strength", "A TensorMechanicsHardening UserObject that defines hardening of the weak-plane compressive strength");
  params.addRangeCheckedParam<unsigned int>("max_NR_iterations", 20, "max_NR_iterations>0", "Maximum number of Newton-Raphson iterations allowed");
  params.addRequiredRangeCheckedParam<Real>("tip_smoother", "tip_smoother>=0", "The cone vertex at shear-stress = 0 will be smoothed by the given amount.  Typical value is 0.1*cohesion");
  params.addParam<bool>("perform_finite_strain_rotations", false, "Tensors are correctly rotated in finite-strain simulations.  For optimal performance you can set this to 'false' if you are only ever using small strains");
  params.addRequiredParam<Real>("smoothing_tol", "Intersections of the yield surfaces will be smoothed by this amount (this is measured in units of stress)");
  params.addRequiredParam<Real>("yield_function_tol", "The return-map process will be deemed to have converged if all yield functions are within yield_function_tol of zero");
  MooseEnum tangent_operator("elastic nonlinear", "nonlinear");
  params.addParam<MooseEnum>("tangent_operator", tangent_operator, "Type of tangent operator to return.  'elastic': return the elasticity tensor.  'nonlinear': return the full consistent tangent operator.");
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
    _elastic_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "elastic_strain"))
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
}

void
ComputeCappedWeakPlaneStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();

  _plastic_strain[_qp].zero();
  _intnl[_qp].assign(2, 0);
  _yf[_qp].assign(3, 0);
  _iter[_qp] = 0.0;
  _linesearch_needed[_qp] = 0.0;
}

void
ComputeCappedWeakPlaneStress::computeQpStress()
{
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
  _intnl[_qp][0] = _intnl_old[_qp][0];
  _intnl[_qp][1] = _intnl_old[_qp][1];
  _iter[_qp] = 0.0;
  _linesearch_needed[_qp] = 0.0;

  Real p_trial = _stress[_qp](2, 2);
  Real q_trial = std::sqrt(Utility::pow<2>(_stress[_qp](2, 0)) + Utility::pow<2>(_stress[_qp](2, 1)));
  yieldFunctions(p_trial, q_trial, _intnl[_qp], _yf[_qp]);

  if ((_yf[_qp][0] < -_smoothing_tol &&
      _yf[_qp][1] < -_smoothing_tol &&
      _yf[_qp][2] < -_smoothing_tol) ||
      yieldF(p_trial, q_trial, _intnl[_qp]) <= _f_tol)
  {
    // elastic
    _plastic_strain[_qp] = _plastic_strain_old[_qp];
    _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
    return;
  }

  //Moose::out << "starting plasticity with p_trial = " << p_trial << " q_trial = " << q_trial << "\n";

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
  int system_size = 3;

  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Ezxzx = _elasticity_tensor[_qp](2, 0, 2, 0);
  const Real trial20 = _stress[_qp](2, 0);
  const Real trial21 = _stress[_qp](2, 1);

  /* initialise p and q using a good guess based on the non-smoothed
   * situation.
   * initialise gaE = 0
   */
  Real p = p_trial;
  Real q = q_trial;
  _intnl[_qp][0] = _intnl_old[_qp][0] + (q_trial - q) / Ezxzx;
  Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
  _intnl[_qp][1] = _intnl_old[_qp][1] + (p_trial - p) / Ezzzz - (q_trial - q) * tanpsi / Ezxzx;
  Real gaE = 0.0;

  f_and_derivs all_q = smoothAllQuantities(p, q, _intnl[_qp]);
  std::vector<Real> rhs(system_size);
  rhs[0] = all_q.f;
  rhs[1] = p - p_trial;
  rhs[2] = q - q_trial;
  Real res2 = rhs[0] * rhs[0] + rhs[1] * rhs[1] + rhs[2] * rhs[2];
  std::vector<double> jac(system_size * system_size);
  while (res2 > _f_tol2 && _iter[_qp] < (float) _max_nr_its)
  {
    // values of variables at the start of this NR step
    const Real gaE_old = gaE;
    const Real p_old = p;
    const Real q_old = q;
    const Real res2_old = res2;

    //Moose::out << "iter = " << _iter[_qp] << " rhs = " << rhs[0] << " " << rhs[1] << " " << rhs[2] << " so res2 = " << res2 << "\n";

    const Real dintnl0_dq = -1.0 / Ezxzx;
    const Real dintnl1_dp = -1.0 / Ezzzz;
    const Real dintnl1_dq = tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dq / Ezxzx;

    // LAPACK gsev stores the matrix in the following way:
    // d(-yieldF)/d(gaE)
    jac[0] = 0;
    // d(-rhs[1])/d(gaE)
    jac[1] = - all_q.dg[0];
    // d(-rhs[2])/d(gaE))
    jac[2] = - Ezxzx * all_q.dg[1] / Ezzzz;
    // d(-yieldF)/d(p)
    jac[3] = - (all_q.df[0] + all_q.df_di[1] * dintnl1_dp);
    // d(-rhs[1])/d(p)
    jac[4] = - 1.0 - gaE * (all_q.d2g[0][0] + all_q.d2g_di[0][1] * dintnl1_dp);
    // d(-rhs[2])/d(p)
    jac[5] = - Ezxzx * gaE / Ezzzz * (all_q.d2g[1][0] + all_q.d2g_di[1][1] * dintnl1_dp);
    // d(-yieldF)/d(q)
    jac[6] = - (all_q.df[1] + all_q.df_di[0] * dintnl0_dq + all_q.df_di[1] * dintnl1_dq);
    // d(-rhs[1])/d(q)
    jac[7] = - gaE * (all_q.d2g[0][1] + all_q.d2g_di[0][0] * dintnl0_dq + all_q.d2g_di[0][1] * dintnl1_dq);
    // d(-rhs[2])/d(q)
    jac[8] = - 1.0 - Ezxzx * gaE / Ezzzz * (all_q.d2g[1][1] + all_q.d2g_di[1][0] * dintnl0_dq + all_q.d2g_di[1][1] * dintnl1_dq);

    /*
    const Real ep = 1E-8;
    Real tmp_tanpsi;
    std::vector<Real> tmp_int(2);
    tmp_int[0] = _intnl_old[_qp][0] + (q_trial - q) / Ezxzx;
    tmp_tanpsi = _tan_psi.value(tmp_int[0]);
    tmp_int[1] = _intnl_old[_qp][1] + (p_trial - (p + ep)) / Ezzzz - (q_trial - q) * tmp_tanpsi / Ezxzx;
    f_and_derivs tmp_ep = smoothAllQuantities(p+ep, q, tmp_int);
    Real tmp_rhs0 = tmp_ep.f;
    Real tmp_rhs1 = (p + ep) - p_trial + gaE * tmp_ep.dg[0];
    Real tmp_rhs2 = q - q_trial + Ezxzx * gaE / Ezzzz * tmp_ep.dg[1];

    Moose::out << "Jac d/dp check.  (FD, ana) pairs should be the same: (" << (tmp_rhs0 - rhs[0])/ep << ", " << -jac[3] << ") (" << (tmp_rhs1 - rhs[1])/ep << ", " << -jac[4] << ") (" << (tmp_rhs2 - rhs[2])/ep << ", " << -jac[5] << ")\n";

    tmp_int[0] = _intnl_old[_qp][0] + (q_trial - (q + ep)) / Ezxzx;
    tmp_tanpsi = _tan_psi.value(tmp_int[0]);
    tmp_int[1] = _intnl_old[_qp][1] + (p_trial - p) / Ezzzz - (q_trial - (q + ep)) * tmp_tanpsi / Ezxzx;
    tmp_ep = smoothAllQuantities(p, q+ep, tmp_int);
    tmp_rhs0 = tmp_ep.f;
    tmp_rhs1 = p - p_trial + gaE * tmp_ep.dg[0];
    tmp_rhs2 = (q + ep) - q_trial + Ezxzx * gaE / Ezzzz * tmp_ep.dg[1];

    Moose::out << "Jac d/dq check.  (FD, ana) pairs should be the same: (" << (tmp_rhs0 - rhs[0])/ep << ", " << -jac[6] << ") (" << (tmp_rhs1 - rhs[1])/ep << ", " << -jac[7] << ") (" << (tmp_rhs2 - rhs[2])/ep << ", " << -jac[8] << ")\n";

    tmp_int[0] = _intnl_old[_qp][0] + (q_trial - q) / Ezxzx;
    tmp_tanpsi = _tan_psi.value(tmp_int[0]);
    tmp_int[1] = _intnl_old[_qp][1] + (p_trial - p) / Ezzzz - (q_trial - q) * tmp_tanpsi / Ezxzx;
    tmp_ep = smoothAllQuantities(p, q, tmp_int);
    tmp_rhs0 = tmp_ep.f;
    tmp_rhs1 = p - p_trial + (gaE + ep) * tmp_ep.dg[0];
    tmp_rhs2 = q - q_trial + Ezxzx * (gaE + ep) / Ezzzz * tmp_ep.dg[1];

    Moose::out << "Jac d/dgaE check.  (FD, ana) pairs should be the same: (" << (tmp_rhs0 - rhs[0])/ep << ", " << -jac[0] << ") (" << (tmp_rhs1 - rhs[1])/ep << ", " << -jac[1] << ") (" << (tmp_rhs2 - rhs[2])/ep << ", " << -jac[2] << ")\n";
    */



    // use LAPACK to solve the linear system
    int nrhs = 1;
    std::vector<int> ipiv(system_size);
    int info;
    LAPACKgesv_(&system_size, &nrhs, &jac[0], &system_size, &ipiv[0], &rhs[0], &system_size, &info);
    if (info != 0)
      mooseError("ComputeCappedWeakPlaneStress: PETSC LAPACK gsev routine returned with error code " << info);

    // extract the solution
    const Real de_gaE = rhs[0];
    const Real de_p = rhs[1];
    const Real de_q = rhs[2];

    Real lam = 1.0; // line-search parameter
    const Real lam_min = 1E-10; // minimum value of lam allowed
    const Real slope = -2.0 * res2_old; // "Numerical Recipes" uses -b*A*x, in order to check for roundoff, but i hope the nrStep would warn if there were problems
    Real tmp_lam; // cached value of lam used in quadratic & cubic line search
    Real f2 = res2_old; // cached value of f = residual2 used in the cubic in the line search
    Real lam2 = lam; // cached value of lam used in the cubic in the line search

    while (true)
    {
      //Moose::out << " trying lam = " << lam;
      // update variables using the current line-search parameter
      gaE = gaE_old + lam * de_gaE;
      p = p_old + lam * de_p;
      q = q_old + lam * de_q;

      // and internal parameters
      _intnl[_qp][0] = _intnl_old[_qp][0] + (q_trial - q) / Ezxzx;
      tanpsi = _tan_psi.value(_intnl[_qp][0]);
      _intnl[_qp][1] = _intnl_old[_qp][1] + (p_trial - p) / Ezzzz - (q_trial - q) * tanpsi / Ezxzx;

      all_q = smoothAllQuantities(p, q, _intnl[_qp]);

      // update rhs for next-time through
      rhs[0] = all_q.f;
      rhs[1] = p - p_trial + gaE * all_q.dg[0];
      rhs[2] = q - q_trial + Ezxzx * gaE / Ezzzz * all_q.dg[1];
      res2 = rhs[0] * rhs[0] + rhs[1] * rhs[1] + rhs[2] * rhs[2];

      //Moose::out << " that gives res2 = " << res2 << " and need " << res2_old + 1E-4 * lam * slope << "  Here rhs0, rhs1, rhs2 = " << rhs[0] << " " << rhs[1] << " " << rhs[2] << "\n";

      if (res2 < res2_old + 1E-4 * lam * slope)
        break;
      else if (lam < lam_min)
      {
        //Moose::out << "FAIL: delgaE = " << de_gaE << " delp = " << de_p << " delq = " << de_q << "\n";
        mooseError("Handle this case!\n");
      }
      else if (lam == 1.0)
      {
        // model as a quadratic
        tmp_lam = - 0.5 * slope / (res2 - res2_old - slope);
      }
      else
      {
        //model as a cubic
        Real rhs1 = res2 - res2_old - lam * slope;
        Real rhs2 = f2 - res2_old - lam2 * slope;
        Real a = (rhs1 / Utility::pow<2>(lam) - rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
        Real b = (-lam2 * rhs1 / Utility::pow<2>(lam) + lam * rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
        if (a == 0.0)
          tmp_lam = -slope / (2.0 * b);
        else
        {
          Real disc = Utility::pow<2>(b) - 3.0 * a * slope;
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

    //Moose::out << "delgaE = " << de_gaE << " delp = " << de_p << " delq = " << de_q << " p = " << p << " q = " << q << " gaE = " << gaE << "\n";


    _iter[_qp] += 1.0;
  }

  if (_iter[_qp] >= (float) _max_nr_its)
    mooseError("NR non-convergence!");

  // success!
  _yf[_qp][0] = 0.0;
  _yf[_qp][1] = 0.0;
  _yf[_qp][2] = 0.0;
  yieldFunctions(p, q, _intnl[_qp], _yf[_qp]); // can remove this line when i know this all works OK

  _stress[_qp](2, 2) = p;
  // stress_xx and stress_yy are sitting at their trial-stress values
  // so need to bring them back via Poisson's ratio
  _stress[_qp](0, 0) -= _elasticity_tensor[_qp](2, 2, 0, 0) * gaE / Ezzzz * all_q.dg[0];
  _stress[_qp](1, 1) -= _elasticity_tensor[_qp](2, 2, 1, 1) * gaE / Ezzzz * all_q.dg[0];
  if (q_trial == 0.0)
    _stress[_qp](2, 0) = _stress[_qp](2, 1) = _stress[_qp](0, 2) = _stress[_qp](1, 2) = 0.0;
  else
  {
    _stress[_qp](2, 0) = _stress[_qp](0, 2) = trial20 * q / q_trial;
    _stress[_qp](2, 1) = _stress[_qp](1, 2) = trial21 * q / q_trial;
  }

  _plastic_strain[_qp] = _plastic_strain_old[_qp] + _strain_increment[_qp] + _elasticity_tensor[_qp].invSymm() * (_stress_old[_qp] - _stress[_qp]);
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];

  if (_tangent_operator_type == elastic)
    return;

  // now form the nonlinear parts of the consistent tangent operator
  const Real dintnl0_dq = -1.0 / Ezxzx;
  const Real dintnl0_dqt = 1.0 / Ezxzx;
  const Real dintnl1_dp = -1.0 / Ezzzz;
  const Real dintnl1_dpt = 1.0 / Ezzzz;
  const Real dintnl1_dq = tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dq / Ezxzx;
  const Real dintnl1_dqt = - tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dqt / Ezxzx;

  all_q = smoothAllQuantities(p, q, _intnl[_qp]);

  std::vector<Real> rhs_cto(2 * system_size);
  // change in p_trial
  rhs_cto[0] = - all_q.df_di[1] * dintnl1_dpt;
  rhs_cto[1] = 1.0 - gaE * all_q.d2g_di[0][1] * dintnl1_dpt;
  rhs_cto[2] = - Ezxzx * gaE / Ezzzz * all_q.d2g_di[1][1] * dintnl1_dpt;
  // change in q_trial
  rhs_cto[3] = - all_q.df_di[0] * dintnl0_dqt - all_q.df_di[1] * dintnl1_dqt;
  rhs_cto[4] = - gaE * (all_q.d2g_di[0][0] * dintnl0_dqt + all_q.d2g_di[0][1] * dintnl1_dqt);
  rhs_cto[5] = 1.0 - Ezxzx * gaE / Ezzzz * (all_q.d2g_di[1][0] * dintnl0_dqt + all_q.d2g_di[1][1] * dintnl1_dqt);

  jac[0] = 0;
  jac[1] = all_q.dg[0];
  jac[2] = Ezxzx * all_q.dg[1] / Ezzzz;
  jac[3] = (all_q.df[0] + all_q.df_di[1] * dintnl1_dp);
  jac[4] = 1.0 + gaE * (all_q.d2g[0][0] + all_q.d2g_di[0][1] * dintnl1_dp);
  jac[5] = Ezxzx * gaE / Ezzzz * (all_q.d2g[1][0] + all_q.d2g_di[1][1] * dintnl1_dp);
  jac[6] = (all_q.df[1] + all_q.df_di[0] * dintnl0_dq + all_q.df_di[1] * dintnl1_dq);
  jac[7] = gaE * (all_q.d2g[0][1] + all_q.d2g_di[0][0] * dintnl0_dq + all_q.d2g_di[0][1] * dintnl1_dq);
  jac[8] = 1.0 + Ezxzx * gaE / Ezzzz * (all_q.d2g[1][1] + all_q.d2g_di[1][0] * dintnl0_dq + all_q.d2g_di[1][1] * dintnl1_dq);

  int nrhs = 2;
  std::vector<int> ipiv(system_size);
  int info;
  LAPACKgesv_(&system_size, &nrhs, &jac[0], &system_size, &ipiv[0], &rhs_cto[0], &system_size, &info);
  if (info != 0)
    mooseError("ComputeCappedWeakPlaneStress: PETSC LAPACK gsev routine returned with error code " << info);

  const Real dgaE_dpt = rhs_cto[0];
  const Real dp_dpt = rhs_cto[1];
  const Real dq_dpt = rhs_cto[2];
  const Real dgaE_dqt = rhs_cto[3];
  const Real dp_dqt = rhs_cto[4];
  const Real dq_dqt = rhs_cto[5];

  //Moose::out << "Solution is rhs= " << rhs_cto[0] << " " << rhs_cto[1] << " " << rhs_cto[2] << " " << rhs_cto[3] << " " << rhs_cto[4] << " " << rhs_cto[5] << "\n";

  // assume stuff about elasticity_tensor

  for (unsigned i = 0; i < 3; ++i)
  {
    const Real dpt_depii = _elasticity_tensor[_qp](2, 2, i, i);
    _Jacobian_mult[_qp](2, 2, i, i) = dp_dpt * dpt_depii;
    _Jacobian_mult[_qp](0, 0, i, i) -= _elasticity_tensor[_qp](2, 2, 0, 0) / Ezzzz * (dgaE_dpt * all_q.dg[0] + gaE * all_q.d2g[0][0] * dp_dpt + gaE * all_q.d2g[0][1] * dq_dpt + gaE * all_q.d2g_di[0][0] * (dintnl0_dq * dq_dpt) + gaE * all_q.d2g_di[0][1] * (dintnl1_dpt + dintnl1_dp * dp_dpt + dintnl1_dq * dq_dpt)) * dpt_depii;
    _Jacobian_mult[_qp](1, 1, i, i) -= _elasticity_tensor[_qp](2, 2, 1, 1) / Ezzzz * (dgaE_dpt * all_q.dg[0] + gaE * all_q.d2g[0][0] * dp_dpt + gaE * all_q.d2g[0][1] * dq_dpt + gaE * all_q.d2g_di[0][0] * (dintnl0_dq * dq_dpt) + gaE * all_q.d2g_di[0][1] * (dintnl1_dpt + dintnl1_dp * dp_dpt + dintnl1_dq * dq_dpt)) * dpt_depii;
    if (q_trial > 0.0)
    {
      _Jacobian_mult[_qp](2, 0, i, i) = _Jacobian_mult[_qp](0, 2, i, i) = trial20 / q_trial * dq_dpt * dpt_depii;
      _Jacobian_mult[_qp](2, 1, i, i) = _Jacobian_mult[_qp](1, 2, i, i) = trial21 / q_trial * dq_dpt * dpt_depii;
    }
  }

  const Real dqt_dep20 = (q_trial == 0.0 ? 1.0 : trial20 / q_trial) * _elasticity_tensor[_qp](2, 0, 2, 0);
  _Jacobian_mult[_qp](2, 2, 2, 0) = _Jacobian_mult[_qp](2, 2, 0, 2) = dp_dqt * dqt_dep20;
  _Jacobian_mult[_qp](0, 0, 2, 0) = _Jacobian_mult[_qp](0, 0, 0, 2) = _Jacobian_mult[_qp](1, 1, 2, 0) = _Jacobian_mult[_qp](1, 1, 0, 2) = - _elasticity_tensor[_qp](2, 2, 0, 0) / Ezzzz * (dgaE_dqt * all_q.dg[0] + gaE * all_q.d2g[0][0] * dp_dqt + gaE * all_q.d2g[0][1] * dq_dqt + gaE * all_q.d2g_di[0][0] * (dintnl0_dqt + dintnl0_dq * dq_dqt) + gaE * all_q.d2g_di[0][1] * (dintnl1_dqt + dintnl1_dp * dp_dqt + dintnl1_dq * dq_dqt)) * dqt_dep20;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    _Jacobian_mult[_qp](0, 2, 0, 2) = _Jacobian_mult[_qp](2, 0, 0, 2) = _Jacobian_mult[_qp](0, 2, 2, 0) = _Jacobian_mult[_qp](2, 0, 2, 0) = _elasticity_tensor[_qp](2, 0, 2, 0) * q / q_trial + trial20 * dq_dqt / q_trial * dqt_dep20 + trial20 * q / q_trial / q_trial * (-dqt_dep20);
    _Jacobian_mult[_qp](1, 2, 0, 2) = _Jacobian_mult[_qp](2, 1, 0, 2) = _Jacobian_mult[_qp](1, 2, 2, 0) = _Jacobian_mult[_qp](2, 1, 2, 0) = trial21 * dq_dqt / q_trial * dqt_dep20 + trial21 * q / q_trial / q_trial * (-dqt_dep20);
  }


  const Real dqt_dep21 = (q_trial == 0.0 ? 1.0 : trial21 / q_trial) * _elasticity_tensor[_qp](2, 1, 2, 1);
  _Jacobian_mult[_qp](2, 2, 2, 1) = _Jacobian_mult[_qp](2, 2, 1, 2) = dp_dqt * dqt_dep21;
  _Jacobian_mult[_qp](0, 0, 2, 1) = _Jacobian_mult[_qp](0, 0, 1, 2) = _Jacobian_mult[_qp](1, 1, 2, 1) = _Jacobian_mult[_qp](1, 1, 1, 2) = - _elasticity_tensor[_qp](2, 2, 0, 0) / Ezzzz * (dgaE_dqt * all_q.dg[0] + gaE * all_q.d2g[0][0] * dp_dqt + gaE * all_q.d2g[0][1] * dq_dqt + gaE * all_q.d2g_di[0][0] * (dintnl0_dqt + dintnl0_dq * dq_dqt) + gaE * all_q.d2g_di[0][1] * (dintnl1_dqt + dintnl1_dp * dp_dqt + dintnl1_dq * dq_dqt)) * dqt_dep21;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    _Jacobian_mult[_qp](1, 2, 1, 2) = _Jacobian_mult[_qp](2, 1, 1, 2) = _Jacobian_mult[_qp](1, 2, 2, 1) = _Jacobian_mult[_qp](2, 1, 2, 1) = _elasticity_tensor[_qp](2, 1, 2, 1) * q / q_trial + trial21 * dq_dqt / q_trial * dqt_dep21 + trial21 * q / q_trial / q_trial * (-dqt_dep21);
    _Jacobian_mult[_qp](0, 2, 1, 2) = _Jacobian_mult[_qp](2, 0, 1, 2) = _Jacobian_mult[_qp](0, 2, 2, 1) = _Jacobian_mult[_qp](2, 0, 2, 1) = trial20 * dq_dqt / q_trial * dqt_dep21 + trial20 * q / q_trial / q_trial * (-dqt_dep21);
  }

}


void
ComputeCappedWeakPlaneStress::yieldFunctions(Real p, Real q, const std::vector<Real> & intnl, std::vector<Real> & yf)
{
  mooseAssert(intnl.size() == 2, "ComputeCappedWeakPlaneStress: yieldFunctions called with intnl size " + Moose::stringify(intnl.size()));
  mooseAssert(yf.size() == 3, "ComputeCappedWeakPlaneStress: yieldFunctions called with yf size " + Moose::stringify(yf.size()));
  yf[0] = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * _tan_phi.value(intnl[0]) - _cohesion.value(intnl[0]);
  yf[1] = p - _tstrength.value(intnl[1]);
  yf[2] = - p - _cstrength.value(intnl[1]);
}

void
ComputeCappedWeakPlaneStress::dyieldFunctions(Real /*p*/, Real q, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & dyf)
{
  mooseAssert(intnl.size() == 2, "ComputeCappedWeakPlaneStress: dyieldFunctions called with intnl size " + Moose::stringify(intnl.size()));
  mooseAssert(dyf.size() == 3, "ComputeCappedWeakPlaneStress: dyieldFunctions called with dyf size " + Moose::stringify(dyf.size()));
  mooseAssert(dyf[0].size() == 2, "ComputeCappedWeakPlaneStress: dyieldFunctions called with dyf[0] size " + Moose::stringify(dyf[0].size()));
  // derivatives wrt p
  dyf[0][0] = _tan_phi.value(intnl[0]);
  dyf[1][0] = 1.0;
  dyf[2][0] = - 1.0;
  // derivatives wrt q
  if (_small_smoother2 == 0.0)
    dyf[0][1] = 1.0;
  else
    dyf[0][1] = q / std::sqrt(Utility::pow<2>(q) + _small_smoother2);
  dyf[1][1] = 0.0;
  dyf[2][1] = 0.0;
}

void
ComputeCappedWeakPlaneStress::dyieldFunctions_di(Real p, Real /*q*/, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & dyf)
{
  mooseAssert(intnl.size() == 2, "ComputeCappedWeakPlaneStress: dyieldFunctions_di called with intnl size " + Moose::stringify(intnl.size()));
  mooseAssert(dyf.size() == 3, "ComputeCappedWeakPlaneStress: dyieldFunctions_di called with dyf size " + Moose::stringify(dyf.size()));
  mooseAssert(dyf[0].size() == 2, "ComputeCappedWeakPlaneStress: dyieldFunctions_di called with dyf[0] size " + Moose::stringify(dyf[0].size()));
  // derivatives wrt intnl[0] (shear plastic strain)
  dyf[0][0] = p * _tan_phi.derivative(intnl[0]) - _cohesion.derivative(intnl[0]);
  dyf[1][0] = 0.0;
  dyf[2][0] = 0.0;
  // derivatives wrt intnl[q] (tensile plastic strain)
  dyf[0][1] = 0.0;
  dyf[1][1] = - _tstrength.derivative(intnl[1]);
  dyf[2][1] = - _cstrength.derivative(intnl[1]);
}

void
ComputeCappedWeakPlaneStress::flowPotential(Real p, Real q, const std::vector<Real> & intnl, std::vector<Real> & g)
{
  mooseAssert(intnl.size() == 2, "ComputeCappedWeakPlaneStress: flowPotential called with intnl size " + Moose::stringify(intnl.size()));
  mooseAssert(g.size() == 3, "ComputeCappedWeakPlaneStress: flowPotential called with g size " + Moose::stringify(g.size()));
  g[0] = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * _tan_psi.value(intnl[0]);
  g[1] = p;
  g[2] = - p;
}

void
ComputeCappedWeakPlaneStress::dflowPotential(Real /*p*/, Real q, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & dg)
{
  mooseAssert(intnl.size() == 2, "ComputeCappedWeakPlaneStress: dflowPotential called with intnl size " + Moose::stringify(intnl.size()));
  mooseAssert(dg.size() == 3, "ComputeCappedWeakPlaneStress: dflowPotential called with dg size " + Moose::stringify(dg.size()));
  mooseAssert(dg[0].size() == 2, "ComputeCappedWeakPlaneStress: dflowPotential called with dg[0] size " + Moose::stringify(dg[0].size()));
  // derivatives wrt p
  dg[0][0] = _tan_psi.value(intnl[0]);
  dg[1][0] = 1.0;
  dg[2][0] = - 1.0;
  // derivatives wrt q
  if (_small_smoother2 == 0.0)
    dg[0][1] = 1.0;
  else
    dg[0][1] = q / std::sqrt(Utility::pow<2>(q) + _small_smoother2);
  dg[1][1] = 0.0;
  dg[2][1] = 0.0;
}

void
ComputeCappedWeakPlaneStress::d2flowPotential_di(Real /*p*/, Real /*q*/, const std::vector<Real> & intnl, std::vector<std::vector<std::vector<Real> > > & d2g)
{
  mooseAssert(intnl.size() == 2, "ComputeCappedWeakPlaneStress: d2flowPotential_di called with intnl size " + Moose::stringify(intnl.size()));
  mooseAssert(d2g.size() == 3, "ComputeCappedWeakPlaneStress: d2flowPotential_di called with d2g size " + Moose::stringify(d2g.size()));
  mooseAssert(d2g[0].size() == 2, "ComputeCappedWeakPlaneStress: d2flowPotential_di called with d2g[0] size " + Moose::stringify(d2g[0].size()));
  mooseAssert(d2g[0][0].size() == 2, "ComputeCappedWeakPlaneStress: d2flowPotential_di called with d2g[0][0] size " + Moose::stringify(d2g[0][0].size()));
  // d(dg/dp)/dintnl[0]
  d2g[0][0][0] = _tan_psi.derivative(intnl[0]);
  d2g[1][0][0] = 0.0;
  d2g[2][0][0] = 0.0;
  // d(dg/dp)/dintnl[1]
  d2g[0][0][1] = 0.0;
  d2g[1][0][1] = 0.0;
  d2g[2][0][1] = 0.0;
  // d(dg/dq)/dintnl[0]
  d2g[0][1][0] = 0.0;
  d2g[1][1][0] = 0.0;
  d2g[2][1][0] = 0.0;
  // d(dg/dq)/dintnl[1]
  d2g[0][1][1] = 0.0;
  d2g[1][1][1] = 0.0;
  d2g[2][1][1] = 0.0;
}

void
ComputeCappedWeakPlaneStress::d2flowPotential(Real /*p*/, Real q, const std::vector<Real> & /*intnl*/, std::vector<std::vector<std::vector<Real> > > & d2g)
{
  mooseAssert(d2g.size() == 3, "ComputeCappedWeakPlaneStress: d2flowPotential called with d2g size " + Moose::stringify(d2g.size()));
  mooseAssert(d2g[0].size() == 2, "ComputeCappedWeakPlaneStress: d2flowPotential called with d2g[0] size " + Moose::stringify(d2g[0].size()));
  mooseAssert(d2g[0][0].size() == 2, "ComputeCappedWeakPlaneStress: d2flowPotential called with d2g[0][0] size " + Moose::stringify(d2g[0][0].size()));
  // d(dg/dp)/dp
  d2g[0][0][0] = 0.0;
  d2g[1][0][0] = 0.0;
  d2g[2][0][0] = 0.0;
  // d(dg/dp)/dq
  d2g[0][0][1] = 0.0;
  d2g[1][0][1] = 0.0;
  d2g[2][0][1] = 0.0;
  // d(dg/dq)/dp
  d2g[0][1][0] = 0.0;
  d2g[1][1][0] = 0.0;
  d2g[2][1][0] = 0.0;
  // d(dg/dq)/dq
  if (_small_smoother2 == 0.0)
    d2g[0][1][1] = 0.0;
  else
    d2g[0][1][1] = _small_smoother2 / std::pow(Utility::pow<2>(q) + _small_smoother2, 1.5);
  d2g[1][1][1] = 0.0;
  d2g[2][1][1] = 0.0;
}

Real
ComputeCappedWeakPlaneStress::yieldF(Real p, Real q, const std::vector<Real> & intnl)
{
  std::vector<Real> yf(3);
  yieldFunctions(p, q, intnl, yf);
  smooth(yf);
  return yf.back();
}

ComputeCappedWeakPlaneStress::f_and_derivs
ComputeCappedWeakPlaneStress::smoothAllQuantities(Real p, Real q, const std::vector<Real> & intnl)
{
  std::vector<Real> yf(3);
  yieldFunctions(p, q, intnl, yf);
  std::vector<std::vector<Real> > dyf(3);
  for (unsigned i = 0; i < 3; ++i)
    dyf[i].resize(2);
  dyieldFunctions(p, q, intnl, dyf);
  std::vector<std::vector<Real> > dyf_di(3);
  for (unsigned i = 0; i < 3; ++i)
    dyf_di[i].resize(2);
  dyieldFunctions_di(p, q, intnl, dyf_di);
  std::vector<std::vector<Real> > dg(3);
  for (unsigned i = 0; i < 3; ++i)
    dg[i].resize(2);
  dflowPotential(p, q, intnl, dg);
  std::vector<std::vector<std::vector<Real> > > d2g(3);
  for (unsigned i = 0; i < 3; ++i)
  {
    d2g[i].resize(2);
    for (unsigned j = 0; j < 2; ++j)
    {
      d2g[i][j].resize(2);
    }
  }
  d2flowPotential(p, q, intnl, d2g);
  std::vector<std::vector<std::vector<Real> > > d2g_di(3);
  for (unsigned i = 0; i < 3; ++i)
  {
    d2g_di[i].resize(2);
    for (unsigned j = 0; j < 2; ++j)
    {
      d2g_di[i][j].resize(2);
    }
  }
  d2flowPotential_di(p, q, intnl, d2g_di);

  std::vector<f_and_derivs> all_quantities;
  for (unsigned i = 0; i < 3; ++i)
    all_quantities.push_back(f_and_derivs(yf[i],
                                          dyf[i],
                                          dyf_di[i],
                                          dg[i],
                                          d2g[i],
                                          d2g_di[i]));

  std::sort(all_quantities.begin(), all_quantities.end());

  /* This is the key to my smoothing strategy.  While the two
   * biggest yield functions are closer to each other than
   * _smoothing_tol, make a linear combination of them:
   * all_quantities[num - 2].f = the second-biggest yield function
   * = all_quantities[num - 1].f + ismoother(all_quantities[num - 2].f - all_quantities[num - 1].f);
   * = biggest yield function + ismoother(second-biggest - biggest)
   * Then pop off the biggest yield function, and repeat this
   * strategy.
   * Of course all the derivatives must also be smoothed.
   * I assume that d(flow potential)/dstress gets smoothed by the *yield function*, viz
   * d(second-biggest-g) = d(biggest-g) + smoother(second-biggest-f - biggest-f)*(d(second-biggest-g) - d(biggest-g))
   * Only time will tell whether this is a good strategy.
   */
  unsigned num = all_quantities.size();
  while (num > 1 && all_quantities[num - 1].f < all_quantities[num - 2].f + _smoothing_tol)
  {
    const Real ism = ismoother(all_quantities[num - 2].f - all_quantities[num - 1].f);
    const Real sm = smoother(all_quantities[num - 2].f - all_quantities[num - 1].f);
    const Real dsm = dsmoother(all_quantities[num - 2].f - all_quantities[num - 1].f);
    for (unsigned i = 0; i < 2; ++i)
    {
      for (unsigned j = 0; j < 2; ++j)
      {
        all_quantities[num - 2].d2g[i][j] = all_quantities[num - 1].d2g[i][j] + dsm * (all_quantities[num - 2].df[j] - all_quantities[num - 1].df[j]) * (all_quantities[num - 2].dg[i] - all_quantities[num - 1].dg[i]) + sm * (all_quantities[num - 2].d2g[i][j] - all_quantities[num - 1].d2g[i][j]);
        all_quantities[num - 2].d2g_di[i][j] = all_quantities[num - 1].d2g_di[i][j] + dsm * (all_quantities[num - 2].df_di[j] - all_quantities[num - 1].df_di[j]) * (all_quantities[num - 2].dg[i] - all_quantities[num - 1].dg[i]) + sm * (all_quantities[num - 2].d2g_di[i][j] - all_quantities[num - 1].d2g_di[i][j]);
      }
    }
    for (unsigned i = 0; i < 2; ++i)
    {
      all_quantities[num - 2].dg[i] = all_quantities[num - 1].dg[i] + sm * (all_quantities[num - 2].dg[i] - all_quantities[num - 1].dg[i]);
      all_quantities[num - 2].df[i] = all_quantities[num - 1].df[i] + sm * (all_quantities[num - 2].df[i] - all_quantities[num - 1].df[i]);
      all_quantities[num - 2].df_di[i] = all_quantities[num - 1].df_di[i] + sm * (all_quantities[num - 2].df_di[i] - all_quantities[num - 1].df_di[i]);
    }
    all_quantities[num - 2].f = all_quantities[num - 1].f + ism;
    all_quantities.pop_back();
    num = all_quantities.size();
  }
  return all_quantities.back();
}


void
ComputeCappedWeakPlaneStress::smooth(std::vector<Real> & f) const
{
  std::sort(f.begin(), f.end());
  unsigned num = f.size();
  while (num > 1 && f[num - 1] < f[num - 2] + _smoothing_tol)
  {
    f[num - 2] = f[num - 1] + ismoother(f[num - 2] - f[num - 1]);
    f.pop_back();
    num = f.size();
  }
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
