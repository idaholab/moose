/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FiniteStrainCPDislocation.h"

template<>
InputParameters validParams<FiniteStrainCPDislocation>()
{
  InputParameters params = validParams<FiniteStrainCPSlipRateRes>();
  params.addParam<unsigned int>("num_internal_var_nss", 2, "Number of vector internal variables of size nss - default 2, rho_m and rho_i");
  params.addParam<unsigned int>("num_internal_var_scalar", 0, "Number of scalar internal variables");
  params.addParam<Real>("zero_tol", 1e-12, "Tolerance for residual check when variable value is zero");
  params.addParam<Real>("penalty_param", 1e-3, "Penalty parameter for linear regularization");
  params.addRequiredParam<Real>("burgers_length", "Length of Burgers vector");
  params.addRequiredParam<Real>("disloc_line_dist", "Dislocation line distance");
  params.addRequiredParam<Real>("jump_freq", "Jump frequency");
  params.addRequiredParam<Real>("active_enthal", "Activation enthalpy");
  params.addRequiredParam<Real>("thermal_resist", "Thermal resistance to slip");
  params.addParam<Real>("exponentp",0.28 , "Exponent p used in flow rule");
  params.addParam<Real>("exponentq", 1.34 , "Exponent q used in flow rule");
  params.addParam<Real>("boltz_const", 1.38065e-20 , "Boltzman constant: Default unit MPa-mm^3");
  params.addParam<Real>("temp", 273.0 , "Temperature in K");
  params.addParam<Real>("rho_m_barrier_factor", 0.3 , "Mobile dislocation barrier strength factor");
  params.addParam<Real>("rho_i_barrier_factor", 0.4 , "Immobile dislocation barrier strength factor");
  params.addParam<Real>("dyn_reco_factor", 329.5 , "Dynamic recovery constant for immobile dislocations");
  params.addParam<Real>("rho_m_capture_radius", 1e-6 ,"Capture radius for mutual annihilation of mobile dislocations: Unit in mm");
  params.addParam<Real>("rho_mult_factor", 0.143 ,"Dislocation multiplication factor");
  params.addParam<Real>("rho_imm_factor", 0.36 ,"Immobilization factor due to dislocations");
  params.addParam<Real>("rho_self_hard_factor", 1.0 ,"Self hardening of dislocations");
  params.addParam<Real>("rho_latent_hard_factor", 0.2 ,"Latent hardening factor");
  params.addParam<bool>("elastic_const_from_tensor", true , "Calculate elastic constants E, nu and G from elasticity tensor");
  params.addParam<Real>("young_mod", 207e3 ,"Young's Modulus: Units in MPa");
  params.addParam<Real>("shear_mod", 79731 ,"Shear modulus in MPa");
  params.addParam<Real>("rho_zero", 0.0 ,"Numerical zero for dislocation densities");

  return params;
}

FiniteStrainCPDislocation::FiniteStrainCPDislocation(const InputParameters & parameters):
    FiniteStrainCPSlipRateRes(parameters),
  _num_internal_var_nss(getParam<unsigned int>("num_internal_var_nss")),
  _num_internal_var_scalar(getParam<unsigned int>("num_internal_var_scalar")),
  _zero_tol(getParam<Real>("zero_tol")),
  _penalty_param(getParam<Real>("penalty_param")),
  _b(getParam<Real>("burgers_length")),
  _lg(getParam<Real>("disloc_line_dist")),
  _jump_freq(getParam<Real>("jump_freq")),
  _enthal(getParam<Real>("active_enthal")),
  _p(getParam<Real>("exponentp")),
  _q(getParam<Real>("exponentq")),
  _s_therm_v(getParam<Real>("thermal_resist")),
  _k(getParam<Real>("boltz_const")),
  _temp(getParam<Real>("temp")),
  _q_p(getParam<Real>("rho_m_barrier_factor")),
  _k_dyn(getParam<Real>("dyn_reco_factor")),
  _r_c(getParam<Real>("rho_m_capture_radius")),
  _k_mul(getParam<Real>("rho_mult_factor")),
  _beta_rho(getParam<Real>("rho_imm_factor")),
  _self_harden(getParam<Real>("rho_self_hard_factor")),
  _latent_harden(getParam<Real>("rho_latent_hard_factor")),
  _elastic_param_flag(getParam<bool>("elastic_const_from_tensor")),
  _young_mod(getParam<Real>("young_mod")),
  _shear_mod(getParam<Real>("shear_mod")),
  _rho_zero(getParam<Real>("rho_zero")),
  _internal_var(declareProperty<std::vector<Real> >("internal_var")),//Internal variables
  _internal_var_old(declarePropertyOld<std::vector<Real> >("internal_var")),//Internal variables
  _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
  _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
  _slip_incr_glide(_nss,0.0),
  _dslipdtau_glide(_nss,0.0),
  _rho_m(_nss,0.0),
  _rho_m_old(_nss,0.0),
  _rho_m_prev(_nss,0.0),
  _rho_i(_nss,0.0),
  _rho_i_old(_nss,0.0),
  _rho_i_prev(_nss,0.0),
  _interaction_matrix(_nss*_nss),
  _s_therm(_nss,0.0),
  _s_atherm(_nss,0.0),
  _rho_m_evol_flag(_nss, true)
{
  if ( _num_internal_var_nss < 2 )
    mooseError("CDDBCCIron Error: num_internal_var_nss should be >= 2");

  for (unsigned int i = 0; i < _nss; ++i)
    for (unsigned int j = 0; j < _nss; ++j)
    {
      _interaction_matrix[i * _nss + j] = _latent_harden;
      if (i == j) _interaction_matrix[i * _nss + j] = _self_harden;
    }

  for (unsigned int i = 0; i < _nss; ++i)
    _s_therm[i] = _s_therm_v;

  if (_elastic_param_flag)
  {
    _shear_mod = _Cijkl(1,2,1,2);
    Real lambda = _Cijkl(1,1,2,2);
    _young_mod = _shear_mod * (3 * lambda + 2 * _shear_mod )/( lambda + _shear_mod);
  }
}

void
FiniteStrainCPDislocation::update_mobile_disloc_density()
{
  Real tot_rho_m = 0.0;
  for (unsigned int i = 0; i < _nss; ++i)
    tot_rho_m += _rho_m_prev[i];

  Real d_self, d_ann, d_imm, lambda_inv;

  for (unsigned int i = 0; i < _nss; ++i)
  {
    // rate of formation of new mobile dislocations by multiplication at existing dislocation segments
    d_self = _k_mul * std::pow(tot_rho_m , 0.5) * std::abs(_slip_incr[i])/_b;
    // rate of mutual annihilation of mobile dislocation segments with opposite Burgers vector
    d_ann = 2.0 * _r_c * _rho_m_prev[i] * std::abs(_slip_incr[i]) / _b;

    lambda_inv = _beta_rho * std::pow(_rho_m_prev[i] + _rho_i_prev[i], 0.5);
    // rate of immobilization of mobile dislocation by trapping at barriers
    d_imm = std::abs(_slip_incr[i]) * lambda_inv / _b ;

    Real drho_m = d_self -  d_ann - d_imm;

    if (update_statevar(&_rho_m[i], &_rho_m_old[i], drho_m, _rho_zero))
    {
#ifdef DEBUG
      mooseWarning("CDDBCCIron: Mobile dislocation density is negative - slip system index = " << i << " Value " << _rho_m[i]);
#endif
      _err_tol = true;
      return;
    }

    _rho_m_evol_flag[i] = true;
    if (std::abs(_rho_m[i] - _rho_m_old[i]) < _zero_tol)
      _rho_m_evol_flag[i] = false;
  }
}

void
FiniteStrainCPDislocation::update_immobile_disloc_density()
{
  Real d_imm, lambda_inv, d_reco;

  for (unsigned int i = 0; i < _nss; ++i)
  {
    lambda_inv = _beta_rho * std::pow(_rho_m_prev[i] + _rho_i_prev[i],0.5);
    // rate of addition of immobile dislocations by trapping of mobile dislocations
    d_imm = std::abs( _slip_incr[i] ) * lambda_inv / _b ;
    // rate of annihilation of immobile dislocations by dynamic recovery
    d_reco = _k_dyn * _rho_i_prev[i] * std::abs( _slip_incr[i] );

    if (!_rho_m_evol_flag[i])
      d_imm = 0.0;

    Real drho_i = d_imm - d_reco;

    if (update_statevar(&_rho_i[i], &_rho_i_old[i], drho_i, _rho_zero))
    {
#ifdef DEBUG
      mooseWarning("FiniteStrainCPDislocation: Immobile dislocation density is negative - slip system index = " << i << " Value " << _rho_i[i]);
#endif
      _err_tol = true;
      return;
    }

  }
}

void
FiniteStrainCPDislocation::update_athermal_resistance()
{
  for (unsigned int i = 0; i < _nss; ++i)
  {
    Real disloc = 0.0;
    for (unsigned int j = 0; j < _nss; ++j)
      disloc += _interaction_matrix[i * _nss + j] * (_rho_m[j] + _rho_i[j]);
    disloc *= _q_p;
    _s_atherm[i] = _shear_mod * _b * std::pow(disloc, 0.5);
  }
}

void
FiniteStrainCPDislocation::getSlipIncrements()
{
  get_glide_increments();
  if(_err_tol)
    return;

  for (unsigned int i = 0; i < _nss; ++i)
    _slip_incr[i] = _slip_incr_glide[i];

  for (unsigned int i = 0; i < _nss; ++i)
    _dslipdtau[i] = _dslipdtau_glide[i];
}



void
FiniteStrainCPDislocation::get_glide_increments()
{
  for (unsigned int i = 0; i < _nss; ++i)
  {
    Real sgn_tau = 1.0;
    if (_tau[i] < 0.0)
      sgn_tau = -1.0;

    Real c = (std::abs(_tau[i]) - _s_atherm[i])/_s_therm[i];
    Real dc_dtau = sgn_tau / _s_therm[i];

    Real sgn_c = 1.0;
    if (c < 0.0)
      sgn_c = -1.0;

    Real v1 = (std::abs(c) + c)/2.0;
    Real dv1_dc = (sgn_c + 1.0)/2.0;

    Real v2 = (std::abs(c) - c)/2.0;
    Real dv2_dc = (sgn_c - 1.0)/2.0;

    Real v1p = std::pow(v1, _p);
    if (v1p > 1.0)
    {
#ifdef DEBUG
      mooseWarning("FiniteStrainCPDislcation: Flow rule upper limit exceeded " << v1p);
#endif
      _err_tol = true;
      return;
    }

    Real c2 = std::pow(1.0 - v1p, _q) + v2/_penalty_param;

    Real dc2_dv1 = 0.0;

    if (v1 > 0.0)
      dc2_dv1 = - _q * _p * std::pow(1.0 - std::pow(v1, _p) , _q - 1.0) * std::pow(v1, _p - 1.0);

    Real dc2_dv2 = 1.0/_penalty_param;

    Real dc2_dc = dc2_dv1 * dv1_dc + dc2_dv2 * dv2_dc;

    Real a = _rho_m[i] * _b * _lg * _jump_freq;
    Real b = _enthal/ (_k * _temp);

    _slip_incr_glide[i] = a * std::exp(-b * c2) * _dt * sgn_tau;
    _dslipdtau_glide[i] = -a * b * std::exp(-b * c2) * sgn_tau * _dt * dc2_dc * dc_dtau;
  }
}

bool
FiniteStrainCPDislocation::update_statevar(Real * var, Real * var_old, Real dvar, Real var_zero)
{
  if (*var_old < var_zero && dvar < 0.0)
    *var = *var_old;
  else
    *var = *var_old + dvar;

  if (*var < 0.0)
    return true;

  return false;
}

void
FiniteStrainCPDislocation::initSlipSysProps()
{
  switch (_intvar_read_type)
  {
  case 0:
    assignSlipSysRes();
    break;
  case 1:
    readFileInitSlipSysRes();
    break;
  default:
    mooseError("Specify internal variable read type from slip_sys_file or slip_sys_res_file");
  }
}

void
FiniteStrainCPDislocation::assignSlipSysRes()
{
  _gss[_qp].resize(_nss,0.0);
  _gss_old[_qp].resize(_nss,0.0);

  _internal_var[_qp].resize( _nss * (_num_internal_var_nss - 1) + _num_internal_var_scalar,0.0);
  _internal_var_old[_qp].resize( _nss * (_num_internal_var_nss - 1) + _num_internal_var_scalar,0.0);

  for (unsigned int i = 0; i < _nss; ++i)
  {
    _gss[_qp][i] = _gss_old[_qp][i] = _slip_sys_props[i * _num_slip_sys_props];

    for (unsigned int j = 1; j < _num_slip_sys_props; ++j)
      _internal_var[_qp][(j-1) * _nss + i] = _internal_var_old[_qp][(j-1) * _nss + i] = _slip_sys_props[i * _num_slip_sys_props + j];
  }

  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
}

void
FiniteStrainCPDislocation::readFileInitSlipSysRes()
{ 
  _gss[_qp].resize(_nss,0.0);
  _gss_old[_qp].resize(_nss,0.0);

  _internal_var[_qp].resize(_nss * (_num_internal_var_nss - 1) + _num_internal_var_scalar,0.0);
  _internal_var_old[_qp].resize(_nss * (_num_internal_var_nss - 1) + _num_internal_var_scalar,0.0);

  MooseUtils::checkFileReadable(_slip_sys_res_prop_file_name);
  
  std::ifstream file;
  file.open(_slip_sys_res_prop_file_name.c_str());

  for (unsigned int i = 0; i < _nss; ++i)
  {
    if (!(file >> _gss[_qp][i]))
      mooseError("CDDBCCIron File read error in function ReadFileInitSlipSysRes");
  
    _gss_old[_qp][i] = _gss[_qp][i];
    for (unsigned int j = 0; j < _num_internal_var_nss - 1; ++j)
    {
      if (!(file >> _internal_var[_qp][j * _nss + i]))
        mooseError("CDDBCCIron File read error in function ReadFileInitSlipSysRes");

      _internal_var_old[_qp][j * _nss + i] = _internal_var[_qp][j * _nss + i];
    }
  }   
  
  file.close();
  
  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
}

void
FiniteStrainCPDislocation::update_slip_system_resistance()
{
  for (unsigned int i = 0; i < _nss; ++i)
  {
    _rho_m_prev[i] = _rho_m[i];
    _rho_i_prev[i] = _rho_i[i];
  }

  update_mobile_disloc_density();
  if(_err_tol)
    return;
  update_immobile_disloc_density();
  if(_err_tol)
    return;
 update_athermal_resistance();
  if(_err_tol)
    return;
}
