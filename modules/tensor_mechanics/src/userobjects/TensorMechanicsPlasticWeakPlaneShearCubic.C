#include "TensorMechanicsPlasticWeakPlaneShearCubic.h"
#include <math.h> // for M_PI

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneShearCubic>()
{
  InputParameters params = validParams<TensorMechanicsPlasticWeakPlaneShear>();
  params.addRequiredRangeCheckedParam<Real>("cohesion", "cohesion>=0", "Weak plane cohesion");
  params.addRequiredRangeCheckedParam<Real>("friction_angle", "friction_angle>=0 & friction_angle<=45", "Weak-plane friction angle in degrees");
  params.addRequiredRangeCheckedParam<Real>("dilation_angle", "dilation_angle>=0", "Weak-plane dilation angle in degrees.  For associative flow use dilation_angle = friction_angle.  Should not be less than friction angle.");
  params.addParam<Real>("cohesion_residual", "Weak plane cohesion at internal_parameter = limit.  If not given, this defaults to cohesion, ie, perfect plasticity");
  params.addParam<Real>("friction_angle_residual", "Weak plane friction angle in degrees at internal_parameter = limit.  If not given, this defaults to friction_angle, ie, perfect plasticity");
  params.addParam<Real>("dilation_angle_residual", "Weak plane dilation angle in degrees at internal_parameter = limit.  If not given, this defaults to dilation_angle, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("cohesion_limit", 1, "cohesion_limit>0", "Cohesion = cubic between cohesion (at zero internal parameter) and cohesion_residual (at internal_parameter = cohesion_limit).");
  params.addRangeCheckedParam<Real>("friction_angle_limit", 1, "friction_angle_limit>0", "friction_angle = cubic between friction_angle (at zero internal parameter) and friction_angle_residual (at internal_parameter = friction_angle_limit).");
  params.addRangeCheckedParam<Real>("dilation_angle_limit", 1, "dilation_angle_limit>0", "dilation_angle =cubic between dilation_angle (at zero internal parameter) and dilation_angle_residual (at internal_parameter = dilation_angle_limit).");
  params.addClassDescription("Non-associative finite-strain weak-plane shear plasticity with hardening/softening governed by a Cubic rule");

  return params;
}

TensorMechanicsPlasticWeakPlaneShearCubic::TensorMechanicsPlasticWeakPlaneShearCubic(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticWeakPlaneShear(name, parameters),
    _cohesion(getParam<Real>("cohesion")),
    _phi(std::tan(getParam<Real>("friction_angle")*M_PI/180.0)),
    _psi(std::tan(getParam<Real>("dilation_angle")*M_PI/180.0)),
    _cohesion_residual(parameters.isParamValid("cohesion_residual") ? getParam<Real>("cohesion_residual") : _cohesion),
    _phi_residual(parameters.isParamValid("friction_angle_residual") ? std::tan(getParam<Real>("friction_angle_residual")*M_PI/180.0) : _phi),
    _psi_residual(parameters.isParamValid("dilation_angle_residual") ? std::tan(getParam<Real>("dilation_angle_residual")*M_PI/180.0) : _psi),
    _cohesion_limit(getParam<Real>("cohesion_limit")),
    _phi_limit(getParam<Real>("friction_angle_limit")),
    _psi_limit(getParam<Real>("dilation_angle_limit")),
    _half_cohesion_limit(0.5*_cohesion_limit),
    _half_phi_limit(0.5*_phi_limit),
    _half_psi_limit(0.5*_psi_limit),
    _alpha_cohesion((_cohesion - _cohesion_residual)/4.0/std::pow(_half_cohesion_limit, 3)),
    _alpha_phi((_phi - _phi_residual)/4.0/std::pow(_half_phi_limit, 3)),
    _alpha_psi((_psi - _psi_residual)/4.0/std::pow(_half_psi_limit, 3)),
    _beta_cohesion(-3.0*_alpha_cohesion*std::pow(_half_cohesion_limit, 2)),
    _beta_phi(-3.0*_alpha_phi*std::pow(_half_phi_limit, 2)),
    _beta_psi(-3.0*_alpha_psi*std::pow(_half_psi_limit, 2))
{
  if (_phi < _psi)
    mooseError("Weak-plane friction angle must not be less than Weak-plane dilation angle");
  if (_cohesion_residual < 0)
    mooseError("Weak-plane residual cohesion must not be negative");
  if (_phi_residual < 0 || _phi_residual > 1.0 || _psi_residual < 0 || _phi_residual < _psi_residual)
    mooseError("Weak-plane residual friction and dilation angles must lie in [0, 45], and dilation_residual <= friction_residual");
}



Real
TensorMechanicsPlasticWeakPlaneShearCubic::cohesion(const Real internal_param) const
{
  if (internal_param <= 0)
    return _cohesion;
  else if (internal_param >= _cohesion_limit)
    return _cohesion_residual;
  else
    return _alpha_cohesion*std::pow(internal_param - _half_cohesion_limit, 3) + _beta_cohesion*(internal_param - _half_cohesion_limit) + 0.5*(_cohesion + _cohesion_residual);
}

Real
TensorMechanicsPlasticWeakPlaneShearCubic::dcohesion(const Real internal_param) const
{
  if (internal_param <= 0)
    return 0.0;
  else if (internal_param >= _cohesion_limit)
    return 0.0;
  else
    return 3*_alpha_cohesion*std::pow(internal_param - _half_cohesion_limit, 2) + _beta_cohesion;
}

Real
TensorMechanicsPlasticWeakPlaneShearCubic::tan_phi(const Real internal_param) const
{
  if (internal_param <= 0)
    return _phi;
  else if (internal_param >= _phi_limit)
    return _phi_residual;
  else
    return _alpha_phi*std::pow(internal_param - _half_phi_limit, 3) + _beta_phi*(internal_param - _half_phi_limit) + 0.5*(_phi + _phi_residual);
}

Real
TensorMechanicsPlasticWeakPlaneShearCubic::dtan_phi(const Real internal_param) const
{
  if (internal_param <= 0)
    return 0.0;
  else if (internal_param >= _phi_limit)
    return 0.0;
  else
    return 3*_alpha_phi*std::pow(internal_param - _half_phi_limit, 2) + _beta_phi;
}

Real
TensorMechanicsPlasticWeakPlaneShearCubic::tan_psi(const Real internal_param) const
{
  if (internal_param <= 0)
    return _psi;
  else if (internal_param >= _psi_limit)
    return _psi_residual;
  else
    return _alpha_psi*std::pow(internal_param - _half_psi_limit, 3) + _beta_psi*(internal_param - _half_psi_limit) + 0.5*(_psi + _psi_residual);
}

Real
TensorMechanicsPlasticWeakPlaneShearCubic::dtan_psi(const Real internal_param) const
{
  if (internal_param <= 0)
    return 0.0;
  else if (internal_param >= _psi_limit)
    return 0.0;
  else
    return 3*_alpha_psi*std::pow(internal_param - _half_psi_limit, 2) + _beta_psi;
}
