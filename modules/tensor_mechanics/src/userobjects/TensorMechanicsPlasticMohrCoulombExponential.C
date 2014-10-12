#include "TensorMechanicsPlasticMohrCoulombExponential.h"
#include <math.h> // for M_PI

template<>
InputParameters validParams<TensorMechanicsPlasticMohrCoulombExponential>()
{
  InputParameters params = validParams<TensorMechanicsPlasticMohrCoulomb>();
  params.addRequiredRangeCheckedParam<Real>("mc_cohesion", "mc_cohesion>=0", "Mohr-Coulomb cohesion");
  params.addRequiredRangeCheckedParam<Real>("mc_friction_angle", "mc_friction_angle>=0 & mc_friction_angle<=60", "Mohr-Coulomb friction angle in degrees");
  params.addRequiredRangeCheckedParam<Real>("mc_dilation_angle", "mc_dilation_angle>=0", "Mohr-Coulomb dilation angle in degrees.  For associative flow use dilation_angle = friction_angle.  Should not be less than friction angle.");
  params.addParam<Real>("mc_cohesion_residual", "Mohr-Coulomb cohesion at infinite hardening.  If not given, this defaults to mc_cohesion, ie, perfect plasticity");
  params.addParam<Real>("mc_friction_angle_residual", "Mohr-Coulomb friction angle in degrees at infinite hardening.  If not given, this defaults to mc_friction_angle, ie, perfect plasticity");
  params.addParam<Real>("mc_dilation_angle_residual", "Mohr-Coulomb dilation angle in degrees at infinite hardening.  If not given, this defaults to mc_dilation_angle, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("mc_cohesion_rate", 0, "mc_cohesion_rate>=0", "Cohesion = mc_cohesion_residual + (mc_cohesion - mc_cohesion_residual)*exp(-mc_cohesion_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("mc_friction_angle_rate", 0, "mc_friction_angle_rate>=0", "friction_angle = mc_friction_angle_residual + (mc_friction_angle - mc_friction_angle_residual)*exp(-mc_friction_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRangeCheckedParam<Real>("mc_dilation_angle_rate", 0, "mc_dilation_angle_rate>=0", "dilation_angle = mc_dilation_angle_residual + (mc_dilation_angle - mc_dilation_angle_residual)*exp(-mc_dilation_angle_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addClassDescription("Non-associative Mohr-Coulomb plasticity with exponential-type of hardening/softening");

  return params;
}

TensorMechanicsPlasticMohrCoulombExponential::TensorMechanicsPlasticMohrCoulombExponential(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticMohrCoulomb(name, parameters),
    _cohesion(getParam<Real>("mc_cohesion")),
    _phi(getParam<Real>("mc_friction_angle")*M_PI/180.0),
    _psi(getParam<Real>("mc_dilation_angle")*M_PI/180.0),
    _cohesion_residual(parameters.isParamValid("mc_cohesion_residual") ? getParam<Real>("mc_cohesion_residual") : _cohesion),
    _phi_residual(parameters.isParamValid("mc_friction_angle_residual") ? getParam<Real>("mc_friction_angle_residual")*M_PI/180.0 : _phi),
    _psi_residual(parameters.isParamValid("mc_dilation_angle_residual") ? getParam<Real>("mc_dilation_angle_residual")*M_PI/180.0 : _psi),
    _cohesion_rate(getParam<Real>("mc_cohesion_rate")),
    _phi_rate(getParam<Real>("mc_friction_angle_rate")),
    _psi_rate(getParam<Real>("mc_dilation_angle_rate"))

{
  if (_phi < _psi)
    mooseError("Mohr-Coulomb friction angle must not be less than Mohr-Coulomb dilation angle");
  if (_cohesion_residual < 0)
    mooseError("Mohr-Coulomb residual cohesion must not be negative");
  if (_phi_residual < 0 || _phi_residual > M_PI/3.0 || _psi_residual < 0 || _phi_residual < _psi_residual)
    mooseError("Mohr-Coulomb residual friction and dilation angles must lie in [0, 60], and dilation_residual <= friction_residual");

  // check Abbo et al's convexity constraint (Eqn c.18 in their paper)
  Real sin_angle = std::sin(std::max(std::max(_phi, _psi), std::max(_phi_residual, _psi_residual)));
  Real rhs = std::sqrt(3)*(35*std::sin(_tt) + 14*std::sin(5*_tt) - 5*std::sin(7*_tt))/16/std::pow(std::cos(_tt), 5)/(11 - 10*std::cos(2*_tt));
  if (rhs <= sin_angle)
    mooseError("Mohr-Coulomb edge smoothing angle is too small and a non-convex yield surface will result.  Please choose a larger value");

}


Real
TensorMechanicsPlasticMohrCoulombExponential::cohesion(const Real internal_param) const
{
  return _cohesion_residual + (_cohesion - _cohesion_residual)*std::exp(-_cohesion_rate*internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombExponential::dcohesion(const Real internal_param) const
{
  return -_cohesion_rate*(_cohesion - _cohesion_residual)*std::exp(-_cohesion_rate*internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombExponential::phi(const Real internal_param) const
{
  return _phi_residual + (_phi - _phi_residual)*std::exp(-_phi_rate*internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombExponential::dphi(const Real internal_param) const
{
  return -_phi_rate*(_phi - _phi_residual)*std::exp(-_phi_rate*internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombExponential::psi(const Real internal_param) const
{
  return _psi_residual + (_psi - _psi_residual)*std::exp(-_psi_rate*internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombExponential::dpsi(const Real internal_param) const
{
  return -_psi_rate*(_psi - _psi_residual)*std::exp(-_psi_rate*internal_param);
}
