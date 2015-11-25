/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FiniteStrainUObasedCP.h"
#include "petscblaslapack.h"

#include "CrystalPlasticitySlipRate.h"
#include "CrystalPlasticitySlipResistance.h"
#include "CrystalPlasticityStateVariable.h"
#include "CrystalPlasticityStateVariableEvolutionRateComponent.h"

template<>
InputParameters validParams<FiniteStrainUObasedCP>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Crystal Plasticity base class: FCC system with power law flow rule implemented");
  params.addParam<Real>("rtol", 1e-6, "Constitutive stress residue relative tolerance");
  params.addParam<Real>("abs_tol", 1e-6, "Constitutive stress residue absolute tolerance");
  params.addParam<Real>("stol", 1e2, "Constitutive slip system resistance residual tolerance");
  params.addParam<Real>("zero_tol", 1e-12, "Tolerance for residual check when variable value is zero");
  params.addParam<unsigned int>("maxiter", 100 , "Maximum number of iterations for stress update");
  params.addParam<unsigned int>("maxiter_state_variable", 100 , "Maximum number of iterations for state variable update");
  MooseEnum tan_mod_options("exact none","none");// Type of read
  params.addParam<MooseEnum>("tan_mod_type", tan_mod_options, "Type of tangent moduli for preconditioner: default elastic");
  params.addParam<bool>("save_euler_angle", false , "Saves the Euler angles as Material Property if true");
  params.addParam<unsigned int>("maximum_substep_iteration", 1, "Maximum number of substep iteration");
  params.addParam<bool>("use_line_search", false, "Use line search in constitutive update");
  params.addParam<Real>("min_line_search_step_size", 0.01, "Minimum line search step size");
  params.addParam<Real>("line_search_tol",0.5,"Line search bisection method tolerance");
  params.addParam<unsigned int>("line_search_maxiter",20,"Line search bisection method maximum number of iteration");
  MooseEnum line_search_method("CUT_HALF BISECTION","CUT_HALF");
  params.addParam<MooseEnum>("line_search_method",line_search_method,"The method used in line search");
  params.addRequiredParam<std::vector<UserObjectName> >("uo_slip_rates", "List of names of user objects that define the slip rates for this material.");
  params.addRequiredParam<std::vector<UserObjectName> >("uo_slip_resistances", "List of names of user objects that define the slip resistances for this material.");
  params.addRequiredParam<std::vector<UserObjectName> >("uo_state_vars", "List of names of user objects that define the state variable for this material.");
  params.addRequiredParam<std::vector<UserObjectName> >("uo_state_var_evol_rate_comps", "List of names of user objects that define the state variable evolution rate components for this material.");
  return params;
}

FiniteStrainUObasedCP::FiniteStrainUObasedCP(const InputParameters & parameters) :
    ComputeStressBase(parameters),
    _num_uo_slip_rates(parameters.get<std::vector<UserObjectName> >("uo_slip_rates").size()),
    _num_uo_slip_resistances(parameters.get<std::vector<UserObjectName> >("uo_slip_resistances").size()),
    _num_uo_state_vars(parameters.get<std::vector<UserObjectName> >("uo_state_vars").size()),
    _num_uo_state_var_evol_rate_comps(parameters.get<std::vector<UserObjectName> >("uo_state_var_evol_rate_comps").size()),
    _rtol(getParam<Real>("rtol")),
    _abs_tol(getParam<Real>("abs_tol")),
    _stol(getParam<Real>("stol")),
    _zero_tol(getParam<Real>("zero_tol")),
    _maxiter(getParam<unsigned int>("maxiter")),
    _maxiterg(getParam<unsigned int>("maxiter_state_variable")),
    _tan_mod_type(getParam<MooseEnum>("tan_mod_type")),
    _save_euler_angle(getParam<bool>("save_euler_angle")),
    _max_substep_iter(getParam<unsigned int>("maximum_substep_iteration")),
    _use_line_search(getParam<bool>("use_line_search")),
    _min_lsrch_step(getParam<Real>("min_line_search_step_size")),
    _lsrch_tol(getParam<Real>("line_search_tol")),
    _lsrch_max_iter(getParam<unsigned int>("line_search_maxiter")),
    _lsrch_method(getParam<MooseEnum>("line_search_method")),
    _fp(declareProperty<RankTwoTensor>("fp")), // Plastic deformation gradient
    _fp_old(declarePropertyOld<RankTwoTensor>("fp")), // Plastic deformation gradient of previous increment
    _pk2(declareProperty<RankTwoTensor>("pk2")), // 2nd Piola Kirchoff Stress
    _pk2_old(declarePropertyOld<RankTwoTensor>("pk2")), // 2nd Piola Kirchoff Stress of previous increment
    _lag_e(declareProperty<RankTwoTensor>("lage")), // Lagrangian strain
    _lag_e_old(declarePropertyOld<RankTwoTensor>("lage")), // Lagrangian strain of previous increment
    _update_rot(declareProperty<RankTwoTensor>("update_rot")), // Rotation tensor considering material rotation and crystal orientation
    _update_rot_old(declarePropertyOld<RankTwoTensor>("update_rot")),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deformation_gradient")),
    _deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _crysrot(getMaterialProperty<RankTwoTensor>("crysrot")),
    _Euler_angles(getMaterialProperty<RealVectorValue>("Euler_angles"))
{
  if (_save_euler_angle)
  {
    _euler_ang = &declareProperty< std::vector<Real> >("euler_ang");
    _euler_ang_old = &declarePropertyOld< std::vector<Real> >("euler_ang");
  }

  _err_tol = false;

  _delta_dfgrd.zero();

  _first_step_iter = false;
  _last_step_iter = false;
  // Initialize variables in the first iteration of substepping
  _first_substep = true;

  // resize the material properties for each userobject
  _mat_prop_slip_rates.resize(_num_uo_slip_rates);
  _mat_prop_slip_resistances.resize(_num_uo_slip_resistances);
  _mat_prop_state_vars.resize(_num_uo_state_vars);
  _mat_prop_state_vars_old.resize(_num_uo_state_vars);
  _mat_prop_state_var_evol_rate_comps.resize(_num_uo_state_var_evol_rate_comps);

  // resize the flow direction
  _flow_direction.resize(_num_uo_slip_rates);

  // resize local state variables
  _state_vars_old.resize(_num_uo_state_vars);
  _state_vars_prev.resize(_num_uo_state_vars);

  // resize user objects
  _uo_slip_rates.resize(_num_uo_slip_rates);
  _uo_slip_resistances.resize(_num_uo_slip_resistances);
  _uo_state_vars.resize(_num_uo_state_vars);
  _uo_state_var_evol_rate_comps.resize(_num_uo_state_var_evol_rate_comps);

  // assign the user objects
  UserObjectInterface uoi(parameters);
  for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
  {
    _uo_slip_rates[i] = &uoi.getUserObjectByName<CrystalPlasticitySlipRate>(parameters.get<std::vector<UserObjectName> >("uo_slip_rates")[i]);
    _mat_prop_slip_rates[i] = &declareProperty< std::vector<Real> >(parameters.get<std::vector<UserObjectName> >("uo_slip_rates")[i]);
    _flow_direction[i] = &declareProperty< std::vector<RankTwoTensor> >(parameters.get<std::vector<UserObjectName> >("uo_slip_rates")[i] + "_flow_direction");
  }

  for (unsigned int i = 0; i < _num_uo_slip_resistances; ++i)
  {
    _uo_slip_resistances[i] = &uoi.getUserObjectByName<CrystalPlasticitySlipResistance>(parameters.get<std::vector<UserObjectName> >("uo_slip_resistances")[i]);
    _mat_prop_slip_resistances[i] = &declareProperty< std::vector<Real> >(parameters.get<std::vector<UserObjectName> >("uo_slip_resistances")[i]);
  }

  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
  {
    _uo_state_vars[i] = &uoi.getUserObjectByName<CrystalPlasticityStateVariable>(parameters.get<std::vector<UserObjectName> >("uo_state_vars")[i]);
    _mat_prop_state_vars[i] = &declareProperty< std::vector<Real> >(parameters.get<std::vector<UserObjectName> >("uo_state_vars")[i]);
    _mat_prop_state_vars_old[i] = &declarePropertyOld< std::vector<Real> >(parameters.get<std::vector<UserObjectName> >("uo_state_vars")[i]);
  }

  for (unsigned int i = 0; i < _num_uo_state_var_evol_rate_comps; ++i)
  {
    _uo_state_var_evol_rate_comps[i] = &uoi.getUserObjectByName<CrystalPlasticityStateVariableEvolutionRateComponent>(parameters.get<std::vector<UserObjectName> >("uo_state_var_evol_rate_comps")[i]);
    _mat_prop_state_var_evol_rate_comps[i] = &declareProperty< std::vector<Real> >(parameters.get<std::vector<UserObjectName> >("uo_state_var_evol_rate_comps")[i]);
  }
}

void FiniteStrainUObasedCP::initQpStatefulProperties()
{
  for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
  {
    (*_mat_prop_slip_rates[i])[_qp].resize(_uo_slip_rates[i]->variableSize());
    (*_flow_direction[i])[_qp].resize(_uo_slip_rates[i]->variableSize());
  }

  for (unsigned int i = 0; i < _num_uo_slip_resistances; ++i)
    (*_mat_prop_slip_resistances[i])[_qp].resize(_uo_slip_resistances[i]->variableSize());

  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
  {
    (*_mat_prop_state_vars[i])[_qp].resize(_uo_state_vars[i]->variableSize());
    (*_mat_prop_state_vars_old[i])[_qp].resize(_uo_state_vars[i]->variableSize());
    _state_vars_old[i].resize(_uo_state_vars[i]->variableSize());
    _state_vars_prev[i].resize(_uo_state_vars[i]->variableSize());
  }

  for (unsigned int i = 0; i < _num_uo_state_var_evol_rate_comps; ++i)
    (*_mat_prop_state_var_evol_rate_comps[i])[_qp].resize(_uo_state_var_evol_rate_comps[i]->variableSize());

  _stress[_qp].zero();

  _fp[_qp].zero();
  _fp[_qp].addIa(1.0);

  _pk2[_qp].zero();
  _pk2_tmp_old.zero();

  _lag_e[_qp].zero();

  _update_rot[_qp].zero();
  _update_rot[_qp].addIa(1.0);

  if (_save_euler_angle)
  {
    (*_euler_ang)[_qp].resize(LIBMESH_DIM);
    (*_euler_ang_old)[_qp].resize(LIBMESH_DIM);
  }

  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
  {
    // Initializes slip system related properties
    _uo_state_vars[i]->initSlipSysProps((*_mat_prop_state_vars[i])[_qp]);
    (*_mat_prop_state_vars_old[i])[_qp] = (*_mat_prop_state_vars[i])[_qp];
  }
}

/**
 * Solves stress residual equation using NR.
 * Updates slip system resistances iteratively.
 */
void FiniteStrainUObasedCP::computeQpStress()
{
  // Depth of substepping; Limited to maximum substep iteration
  unsigned int substep_iter = 1;
  // Calculated from substep_iter as 2^substep_iter
  unsigned int num_substep = 1;
  // Store original _dt; Reset at the end of solve
  Real dt_original = _dt;
  // Initialize variables at substep_iter = 1
  _first_substep = true;

  if (_max_substep_iter > 1)
  {
    _dfgrd_tmp_old = _deformation_gradient_old[_qp];
    if (_dfgrd_tmp_old.det() == 0)
      _dfgrd_tmp_old.addIa(1.0);

    _delta_dfgrd = _deformation_gradient[_qp] - _dfgrd_tmp_old;
    _err_tol = true;// Indicator to continue substepping
  }

  // Saves the old stateful properties that is modified during sub stepping
  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
  {
    _state_vars_old[i] = (*_mat_prop_state_vars_old[i])[_qp];
  }

  // Substepping loop
  while (_err_tol && _max_substep_iter > 1)
  {
    _dt = dt_original/num_substep;

    for (unsigned int istep = 0; istep < num_substep; ++istep)
    {
      _first_step_iter = false;
      if (istep == 0)
        _first_step_iter = true;

      _last_step_iter = false;
      if (istep == num_substep - 1)
        _last_step_iter = true;

      _dfgrd_scale_factor = (static_cast<Real>(istep)+1)/num_substep;

      preSolveQp();
      solveQp();

      if (_err_tol)
      {
        substep_iter++;
        num_substep*=2;
        break;
      }
    }

    // Prevent reinitialization
    _first_substep = false;

    // Reset dt
    _dt = dt_original;

#ifdef DEBUG
    if (substep_iter > _max_substep_iter)
      mooseWarning("FiniteStrainUObasedCP: Failure with substepping");
#endif

    // Evaluate variables after successful solve or indicate failure
    if (!_err_tol || substep_iter > _max_substep_iter)
      postSolveQp();
  }

  // No substepping
  if (_max_substep_iter == 1)
  {
    preSolveQp();
    solveQp();
    postSolveQp();
  }
}

void
FiniteStrainUObasedCP::preSolveQp()
{
  // Initialize variable
  if (_first_substep)
  {
    // Initializes jacobian for preconditioner
    _Jacobian_mult[_qp].zero();

    for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
      _uo_slip_rates[i]->calcFlowDirection(_qp, (*_flow_direction[i])[_qp]);
  }

  if (_max_substep_iter == 1)
    // Without substepping
    _dfgrd_tmp = _deformation_gradient[_qp];
  else
    _dfgrd_tmp = _dfgrd_scale_factor * _delta_dfgrd + _dfgrd_tmp_old;

  _err_tol = false;
}

void
FiniteStrainUObasedCP::solveQp()
{
  preSolveStatevar();
  solveStatevar();
  if (_err_tol)
    return;
  postSolveStatevar();
}

void
FiniteStrainUObasedCP::postSolveQp()
{
  // Restores the the old stateful properties after a successful solve
  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
    (*_mat_prop_state_vars_old[i])[_qp] = _state_vars_old[i];

  if (_err_tol)
  {
    _err_tol = false;
    mooseError("FiniteStrainUObasedCP: Constitutive failure");
  }
  else
  {
    _stress[_qp] = _fe * _pk2[_qp] * _fe.transpose()/_fe.det();

    // Calculate jacobian for preconditioner
    _Jacobian_mult[_qp] += calcTangentModuli();

    RankTwoTensor iden;
    iden.addIa(1.0);

    _lag_e[_qp] = _deformation_gradient[_qp].transpose() * _deformation_gradient[_qp] - iden;
    _lag_e[_qp] = _lag_e[_qp] * 0.5;

    RankTwoTensor rot;
    // Calculate material rotation
    rot = get_current_rotation(_deformation_gradient[_qp]);
    _update_rot[_qp] = rot * _crysrot[_qp];

    if (_save_euler_angle)
      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
        (*_euler_ang)[_qp][i] = _Euler_angles[_qp](i);
  }
}

void
FiniteStrainUObasedCP::preSolveStatevar()
{
  if (_max_substep_iter == 1)
    _first_step_iter = true;

  if (_first_step_iter)
  {
    for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
      (*_mat_prop_state_vars[i])[_qp] = (*_mat_prop_state_vars_old[i])[_qp] = _state_vars_old[i];
  }
  else
  {
    for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
      (*_mat_prop_state_vars[i])[_qp] = (*_mat_prop_state_vars_old[i])[_qp];
  }

  for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
    _uo_slip_resistances[i]->calcSlipResistance(_qp, (*_mat_prop_slip_resistances[i])[_qp]);
}

void
FiniteStrainUObasedCP::solveStatevar()
{
  unsigned int iterg;
  bool iter_flag = true;

  iterg = 0;
  // Check for slip system resistance update tolerance
  while (iter_flag && iterg < _maxiterg)
  {
    preSolveStress();
    solveStress();
    if (_err_tol)
      return;
    postSolveStress();

    // Update slip system resistance
    updateSlipSystemResistance();

    if (_err_tol)
      return;

    iter_flag = getIterFlagVar();
    iterg++;
  }

  if (iterg == _maxiterg)
  {
#ifdef DEBUG
    mooseWarning("FiniteStrainUObasedCP: Hardness Integration error\n");
#endif
    _err_tol = true;
  }
}

bool
FiniteStrainUObasedCP::getIterFlagVar()
{
  Real diff;

  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
  {
    unsigned int n = (*_mat_prop_state_vars[i])[_qp].size();
    for (unsigned j = 0; j < n; j++)
    {
      diff = std::abs((*_mat_prop_state_vars[i])[_qp][j] - _state_vars_prev[i][j]);// Calculate increment size
      if (std::abs((*_mat_prop_state_vars_old[i])[_qp][j]) < _zero_tol && diff > _zero_tol)
        return true;
      if (std::abs((*_mat_prop_state_vars_old[i])[_qp][j]) >  _zero_tol && diff > _stol * std::abs((*_mat_prop_state_vars_old[i])[_qp][j]))
        return true;
    }
  }
  return false;
}

void
FiniteStrainUObasedCP::postSolveStatevar()
{
  if (_max_substep_iter == 1)
    _last_step_iter = true;

  if (!_last_step_iter)
  {
    for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
      (*_mat_prop_state_vars_old[i])[_qp] = (*_mat_prop_state_vars[i])[_qp];
  }
}

void
FiniteStrainUObasedCP::preSolveStress()
{
  if (_max_substep_iter == 1)
  {
    // No substepping
    _pk2[_qp] = _pk2_old[_qp];
    _fp_old_inv = _fp_old[_qp].inverse();
    _fp_inv = _fp_old_inv;
    _fp_prev_inv = _fp_inv;
  }
  else
  {
    if (_first_step_iter)
    {
      _pk2[_qp] = _pk2_tmp_old = _pk2_old[_qp];
      _fp_old_inv = _fp_old[_qp].inverse();
    }
    else
      _pk2[_qp] = _pk2_tmp_old;

    _fp_inv = _fp_old_inv;
    _fp_prev_inv = _fp_inv;
  }
}

void
FiniteStrainUObasedCP::solveStress()
{
  unsigned int iter = 0;
  RankTwoTensor resid, dpk2;
  RankFourTensor jac;
  Real rnorm, rnorm0, rnorm_prev;

  // Calculate stress residual
  calcResidJacob(resid, jac);
  if (_err_tol)
  {
#ifdef DEBUG
    mooseWarning("FiniteStrainUObasedCP: Slip increment exceeds tolerance - Element number " << _current_elem->id() << " Gauss point = " << _qp);
#endif
    return;
  }

  rnorm = resid.L2norm();
  rnorm0 = rnorm;

  // Check for stress residual tolerance
  while (rnorm > _rtol * rnorm0 && rnorm0 > _abs_tol && iter <  _maxiter)
  {
    // Calculate stress increment
    dpk2 = - jac.invSymm() * resid;
    _pk2[_qp] = _pk2[_qp] + dpk2;
    calcResidJacob(resid, jac);
    // update _fp_prev_inv
    internalVariableUpdateNRiteration();

    if (_err_tol)
    {
#ifdef DEBUG
      mooseWarning("FiniteStrainUObasedCP: Slip increment exceeds tolerance - Element number " << _current_elem->id() << " Gauss point = " << _qp);
#endif
      return;
    }

    rnorm_prev = rnorm;
    rnorm = resid.L2norm();

    if (_use_line_search && rnorm > rnorm_prev && !lineSearchUpdate(rnorm_prev, dpk2))
    {
#ifdef DEBUG
      mooseWarning("FiniteStrainUObasedCP: Failed with line search");
#endif
      _err_tol = true;
      return;
    }

    if (_use_line_search)
      rnorm = resid.L2norm();

    iter++;
  }

  if (iter >= _maxiter)
  {
#ifdef DEBUG
    mooseWarning("FiniteStrainUObasedCP: Stress Integration error rmax = " << rnorm);
#endif
    _err_tol = true;
  }
}

void
FiniteStrainUObasedCP::postSolveStress()
{
  if (_max_substep_iter == 1)
    // No substepping
    _fp[_qp] = _fp_inv.inverse();
  else
  {
    if (_last_step_iter)
      _fp[_qp] = _fp_inv.inverse();
    else
    {
      _fp_old_inv = _fp_inv;
      _pk2_tmp_old = _pk2[_qp];
    }
  }
}

// Update slip system resistance. Overide to incorporate new slip system resistance laws
void
FiniteStrainUObasedCP::updateSlipSystemResistance()
{
  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
    _state_vars_prev[i] = (*_mat_prop_state_vars[i])[_qp];

  for (unsigned int i = 0; i < _num_uo_state_var_evol_rate_comps; ++i)
    _uo_state_var_evol_rate_comps[i]->calcStateVariableEvolutionRateComponent(_qp, (*_mat_prop_state_var_evol_rate_comps[i])[_qp]);

  for (unsigned int i = 0; i < _num_uo_state_vars; ++i)
  {
    if (!_uo_state_vars[i]->updateStateVariable(_qp, _dt, (*_mat_prop_state_vars[i])[_qp]))
      _err_tol = true;
  }

  for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
    _uo_slip_resistances[i]->calcSlipResistance(_qp, (*_mat_prop_slip_resistances[i])[_qp]);
}

// Calculates stress residual equation and jacobian
void
FiniteStrainUObasedCP::calcResidJacob(RankTwoTensor & resid, RankFourTensor & jac)
{
  calcResidual(resid);
  if (_err_tol)
    return;
  calcJacobian(jac);
}

void
FiniteStrainUObasedCP::getSlipIncrements()
{
  for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
  {
    if (!_uo_slip_rates[i]->calcSlipRate(_qp, _dt, (*_mat_prop_slip_rates[i])[_qp]))
    {
      _err_tol = true;
      return;
    }
  }
}

void
FiniteStrainUObasedCP::calcResidual(RankTwoTensor &resid)
{
  RankTwoTensor iden, ce, ee, ce_pk2, eqv_slip_incr, pk2_new;

  iden.zero();
  iden.addIa(1.0);

  eqv_slip_incr.zero();

  // _fp_inv  ==> _fp_prev_inv
  _fe = _dfgrd_tmp * _fp_prev_inv;

  getSlipIncrements();

  if (_err_tol)
    return;

  for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
    for (unsigned int j = 0; j < _uo_slip_rates[i]->variableSize(); ++j)
      eqv_slip_incr += (*_flow_direction[i])[_qp][j] * (*_mat_prop_slip_rates[i])[_qp][j] * _dt;

  eqv_slip_incr = iden - eqv_slip_incr;
  _fp_inv = _fp_old_inv * eqv_slip_incr;
  _fe = _dfgrd_tmp * _fp_inv;

  ce = _fe.transpose() * _fe;
  ee = ce - iden;
  ee *= 0.5;

  pk2_new = _elasticity_tensor[_qp] * ee;

  resid = _pk2[_qp] - pk2_new;
}

void
FiniteStrainUObasedCP::calcJacobian(RankFourTensor & jac)
{
  RankFourTensor dfedfpinv, deedfe, dfpinvdpk2;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        dfedfpinv(i,j,k,j) = _dfgrd_tmp(i,k);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        deedfe(i,j,k,i) = deedfe(i,j,k,i) + _fe(k,j) * 0.5;
        deedfe(i,j,k,j) = deedfe(i,j,k,j) + _fe(k,i) * 0.5;
      }

  for (unsigned int i = 0; i < _num_uo_slip_rates; ++i)
  {
    unsigned int nss = _uo_slip_rates[i]->variableSize();
    std::vector<RankTwoTensor> dtaudpk2(nss), dfpinvdslip(nss);
    std::vector<Real> dslipdtau;
    dslipdtau.resize(nss);
    _uo_slip_rates[i]->calcSlipRateDerivative(_qp, _dt, dslipdtau);
    for (unsigned int j = 0; j < nss; j++)
    {
      dtaudpk2[j] = (*_flow_direction[i])[_qp][j];
      dfpinvdslip[j] = - _fp_old_inv * (*_flow_direction[i])[_qp][j];
    }

    for (unsigned int j = 0; j < nss; j++)
      dfpinvdpk2 += (dfpinvdslip[j] * dslipdtau[j] * _dt).outerProduct(dtaudpk2[j]);
  }
  jac = RankFourTensor::IdentityFour() - (_elasticity_tensor[_qp] * deedfe * dfedfpinv * dfpinvdpk2);
}

// Calls getMatRot to perform RU factorization of a tensor.
RankTwoTensor
FiniteStrainUObasedCP::get_current_rotation(const RankTwoTensor & a)
{
  return getMatRot(a);
}

// Performs RU factorization of a tensor
RankTwoTensor
FiniteStrainUObasedCP::getMatRot(const RankTwoTensor & a)
{
  RankTwoTensor rot;
  RankTwoTensor c, diag, evec;
  PetscScalar cmat[LIBMESH_DIM][LIBMESH_DIM], work[10];
  PetscReal w[LIBMESH_DIM];
  PetscBLASInt nd = LIBMESH_DIM,
               lwork = 10,
               info;

  c = a.transpose() * a;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      cmat[i][j] = c(i,j);

  LAPACKsyev_("V", "U", &nd, &cmat[0][0], &nd, w, work, &lwork, &info);

  if (info != 0)
    mooseError("FiniteStrainUObasedCP: DSYEV function call in getMatRot function failed");

  diag.zero();

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    diag(i,i) = std::pow(w[i], 0.5);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      evec(i,j) = cmat[i][j];

  rot = a * ((evec.transpose() * diag * evec).inverse());

  return rot;
}

// Calculates tangent moduli which is used for global solve
void
FiniteStrainUObasedCP::computeQpElasticityTensor()
{

}

ElasticityTensorR4
FiniteStrainUObasedCP::calcTangentModuli()
{
  ElasticityTensorR4 tan_mod;

  switch (_tan_mod_type)
  {
    case 0:
      tan_mod = elastoPlasticTangentModuli();
      break;
    default:
      tan_mod = elasticTangentModuli();
  }

  return tan_mod;
}

ElasticityTensorR4
FiniteStrainUObasedCP::elastoPlasticTangentModuli()
{
  ElasticityTensorR4 tan_mod;
  RankTwoTensor pk2fet, fepk2;
  RankFourTensor deedfe, dsigdpk2dfe;

  // Fill in the matrix stiffness material property
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        deedfe(i,j,k,i) = deedfe(i,j,k,i) + _fe(k,j) * 0.5;
        deedfe(i,j,k,j) = deedfe(i,j,k,j) + _fe(k,i) * 0.5;
      }

  dsigdpk2dfe = _fe.mixedProductIkJl(_fe) * _elasticity_tensor[_qp] * deedfe;

  pk2fet = _pk2[_qp] * _fe.transpose();
  fepk2 = _fe * _pk2[_qp];

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
      {
        tan_mod(i,j,i,l) = tan_mod(i,j,i,l) + pk2fet(l,j);
        tan_mod(i,j,j,l) = tan_mod(i,j,j,l) + fepk2(i,l);
      }

  tan_mod += dsigdpk2dfe;

  Real je = _fe.det();
  if (je > 0.0)
    tan_mod /= je;

  return tan_mod;
}

ElasticityTensorR4
FiniteStrainUObasedCP::elasticTangentModuli()
{
  // update jacobian_mult
  return _elasticity_tensor[_qp];
}

bool
FiniteStrainUObasedCP::lineSearchUpdate(const Real rnorm_prev, const RankTwoTensor dpk2)
{
  switch (_lsrch_method)
  {
    case 0: // CUT_HALF
    {
      Real rnorm;
      RankTwoTensor resid;
      Real step = 1.0;

      do
      {
        _pk2[_qp] = _pk2[_qp] - step * dpk2;
        step /= 2.0;
        _pk2[_qp] = _pk2[_qp] + step * dpk2;

        calcResidual(resid);
        rnorm = resid.L2norm();
      }
      while (rnorm > rnorm_prev && step > _min_lsrch_step);

      // has norm improved or is the step still above minumum search step size?
      return (rnorm <= rnorm_prev || step > _min_lsrch_step);
    }

    case 1: // BISECTION
    {
      unsigned int count = 0;
      Real step_a = 0.0;
      Real step_b = 1.0;
      Real step = 1.0;
      Real s_m = 1000.0;
      Real rnorm = 1000.0;

      RankTwoTensor resid;
      calcResidual(resid);
      Real s_b = resid.doubleContraction(dpk2);
      Real rnorm1 = resid.L2norm();
      _pk2[_qp] = _pk2[_qp] - dpk2;
      calcResidual(resid);
      Real s_a = resid.doubleContraction(dpk2);
      Real rnorm0 = resid.L2norm();
      _pk2[_qp] = _pk2[_qp] + dpk2;

      if ((rnorm1/rnorm0) < _lsrch_tol || s_a*s_b > 0){
        calcResidual(resid);
        return true;
      }

      while ((rnorm/rnorm0) > _lsrch_tol && count < _lsrch_max_iter)
      {
        _pk2[_qp] = _pk2[_qp] - step*dpk2;
        step = 0.5 * (step_b + step_a);
        _pk2[_qp] = _pk2[_qp] + step*dpk2;
        calcResidual(resid);
        s_m = resid.doubleContraction(dpk2);
        rnorm = resid.L2norm();

        if (s_m*s_a < 0.0){
          step_b = step;
          s_b = s_m;
        }
        if (s_m*s_b < 0.0){
          step_a = step;
          s_a = s_m;
        }
        count++;
      }

      // below tolerance and max iterations?
      return  ((rnorm/rnorm0) < _lsrch_tol && count < _lsrch_max_iter);
    }

    default:
      mooseError("Line search method is not provided.");
  }
}

void
FiniteStrainUObasedCP::internalVariableUpdateNRiteration()
{
  _fp_prev_inv = _fp_inv;
}
