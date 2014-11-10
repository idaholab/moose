#include "TensorMechanicsPlasticWeakPlaneShearGaussian.h"
#include <math.h> // for M_PI

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneShearGaussian>()
{
  InputParameters params = validParams<TensorMechanicsPlasticWeakPlaneShear>();
  params.addRequiredRangeCheckedParam<Real>("cohesion", "cohesion>=0", "Weak plane cohesion");
  params.addRequiredRangeCheckedParam<Real>("friction_angle", "friction_angle>=0 & friction_angle<=45", "Weak-plane friction angle in degrees");
  params.addRequiredRangeCheckedParam<Real>("dilation_angle", "dilation_angle>=0", "Weak-plane dilation angle in degrees.  For associative flow use dilation_angle = friction_angle.  Should not be less than friction angle.");
  params.addParam<Real>("cohesion_residual", "Weak plane cohesion at infinite hardening.  If not given, this defaults to cohesion, ie, perfect plasticity");
  params.addParam<Real>("friction_angle_residual", "Weak-plane friction angle in degrees at infinite hardening.  If not given, this defaults to friction_angle, ie, perfect plasticity");
  params.addParam<Real>("dilation_angle_residual", "Weak-plane dilation angle in degrees at infinite hardening.  If not given, this defaults to dilation_angle, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("cohesion_rate", 0, "cohesion_rate>=0", "Cohesion = cohesion_residual + (cohesion - cohesion_residual)*exp(-cohesion_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("friction_angle_rate", 0, "friction_angle_rate>=0", "tan(friction_angle) = tan(friction_angle_residual) + (tan(friction_angle) - tan(friction_angle_residual))*exp(-friction_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("dilation_angle_rate", 0, "dilation_angle_rate>=0", "tan(dilation_angle) = tan(dilation_angle_residual) + (tan(dilation_angle) - tan(dilation_angle_residual))*exp(-dilation_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addClassDescription("Non-associative finite-strain weak-plane shear plasticity with hardening/softening governed by a Gaussian rule");

  return params;
}

TensorMechanicsPlasticWeakPlaneShearGaussian::TensorMechanicsPlasticWeakPlaneShearGaussian(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticWeakPlaneShear(name, parameters),
    _cohesion(getParam<Real>("cohesion")),
    _tan_phi(std::tan(getParam<Real>("friction_angle")*M_PI/180.0)),
    _tan_psi(std::tan(getParam<Real>("dilation_angle")*M_PI/180.0)),
    _cohesion_residual(parameters.isParamValid("cohesion_residual") ? getParam<Real>("cohesion_residual") : _cohesion),
    _tan_phi_residual(parameters.isParamValid("friction_angle_residual") ? std::tan(getParam<Real>("friction_angle_residual")*M_PI/180.0) : _tan_phi),
    _tan_psi_residual(parameters.isParamValid("dilation_angle_residual") ? std::tan(getParam<Real>("dilation_angle_residual")*M_PI/180.0) : _tan_psi),
    _cohesion_rate(getParam<Real>("cohesion_rate")),
    _tan_phi_rate(getParam<Real>("friction_angle_rate")),
    _tan_psi_rate(getParam<Real>("dilation_angle_rate"))
{
  if (_tan_phi < _tan_psi)
    mooseError("Weak-plane friction angle must not be less than weak-plane dilation angle");
  if (_cohesion_residual < 0)
    mooseError("Weak-plane residual cohesion must not be negative");
  if (_tan_phi_residual < 0 || _tan_phi_residual > 1 || _tan_psi_residual < 0 || _tan_phi_residual < _tan_psi_residual)
    mooseError("Weak-plane residual friction and dilation angles must lie in [0, 45], and dilation_residual <= friction_residual");
}



Real
TensorMechanicsPlasticWeakPlaneShearGaussian::cohesion(const Real internal_param) const
{
  if (internal_param >= 0)
    return _cohesion_residual + (_cohesion - _cohesion_residual)*std::exp(-std::pow(_cohesion_rate*internal_param, 2));
  else
    return _cohesion;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::dcohesion(const Real internal_param) const
{
  if (internal_param >= 0)
    return -2.0*std::pow(_cohesion_rate, 2)*internal_param*(_cohesion - _cohesion_residual)*std::exp(-std::pow(_cohesion_rate*internal_param, 2));
  else
    return 0.0;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::tan_phi(const Real internal_param) const
{
  if (internal_param >= 0)
    return _tan_phi_residual + (_tan_phi - _tan_phi_residual)*std::exp(-std::pow(_tan_phi_rate*internal_param, 2));
  else
    return _tan_phi;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::dtan_phi(const Real internal_param) const
{
  if (internal_param >= 0)
    return -2*std::pow(_tan_phi_rate, 2)*internal_param*(_tan_phi - _tan_phi_residual)*std::exp(-std::pow(_tan_phi_rate*internal_param, 2));
  else
    return 0.0;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::tan_psi(const Real internal_param) const
{
  if (internal_param >= 0)
    return _tan_psi_residual + (_tan_psi - _tan_psi_residual)*std::exp(-std::pow(_tan_psi_rate*internal_param, 2));
  else
    return _tan_psi;
}

Real
TensorMechanicsPlasticWeakPlaneShearGaussian::dtan_psi(const Real internal_param) const
{
  if (internal_param >= 0)
    return -2*std::pow(_tan_psi_rate, 2)*internal_param*(_tan_psi - _tan_psi_residual)*std::exp(-std::pow(_tan_psi_rate*internal_param, 2));
  else
    return 0.0;
}
