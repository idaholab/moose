#include "OneDEnergyFlux.h"

template<>
InputParameters validParams<OneDEnergyFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("area", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addRequiredCoupledVar("rhouA", "momentum multiplied by area");
  params.addRequiredCoupledVar("u", "velocity");
  params.addRequiredCoupledVar("enthalpy", "enthalpy");
  params.addParam<bool>("is_liquid", true, "True for liquid, false for vapor");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<MaterialPropertyName>("pressure", "Pressure");
  params.addRequiredParam<MaterialPropertyName>("dp_darhoA", "Derivative of pressure w.r.t. density");
  params.addRequiredParam<MaterialPropertyName>("dp_darhouA", "Derivative of pressure w.r.t. momentum");
  params.addRequiredParam<MaterialPropertyName>("dp_darhoEA", "Derivative of pressure w.r.t. total energy");
  params.addParam<MaterialPropertyName>("dp_dbeta", "Derivative of pressure w.r.t. beta");
  params.addParam<MaterialPropertyName>("daL_dbeta", "Derivative of alphaL w.r.t. beta");

  return params;
}

OneDEnergyFlux::OneDEnergyFlux(const InputParameters & parameters) :
    Kernel(parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _rhouA(coupledValue("rhouA")),
    _area(coupledValue("area")),
    _u_vel(coupledValue("u")),
    _enthalpy(coupledValue("enthalpy")),
    _pressure(getMaterialProperty<Real>("pressure")),
    _dp_darhoA(getMaterialProperty<Real>("dp_darhoA")),
    _dp_darhouA(getMaterialProperty<Real>("dp_darhouA")),
    _dp_darhoEA(getMaterialProperty<Real>("dp_darhoEA")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _has_beta(isCoupled("beta")),
    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _dp_dbeta(_has_beta ? &getMaterialProperty<Real>("dp_dbeta") : NULL),
    _daL_dbeta(_has_beta ? &getMaterialProperty<Real>("daL_dbeta") : NULL),
    _alpha(coupledValue("alpha"))
{
}

OneDEnergyFlux::~OneDEnergyFlux()
{
}

Real
OneDEnergyFlux::computeQpResidual()
{
  return -_rhouA[_qp] * _enthalpy[_qp] * _grad_test[_i][_qp](0);
}

Real
OneDEnergyFlux::computeQpJacobian()
{
  Real A33 = _u_vel[_qp] * (1. + _alpha[_qp] * _dp_darhoEA[_qp] * _area[_qp]);
  return -A33 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDEnergyFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    Real A31 = _u_vel[_qp] * (_alpha[_qp] * _dp_darhoA[_qp] * _area[_qp] - _enthalpy[_qp]);
    return -A31 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _rhouA_var_number)
  {
    Real A32 = _u_vel[_qp] * _alpha[_qp] * _dp_darhouA[_qp] * _area[_qp] + _enthalpy[_qp];
    return -A32 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _beta_var_number)
  {
    return -(_u_vel[_qp] * (_sign * _pressure[_qp] * (*_daL_dbeta)[_qp] + _alpha[_qp] * (*_dp_dbeta)[_qp])) * _area[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
