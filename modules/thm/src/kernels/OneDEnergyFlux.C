#include "OneDEnergyFlux.h"

template <>
InputParameters
validParams<OneDEnergyFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("area", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addRequiredCoupledVar("rhouA", "momentum multiplied by area");
  params.addRequiredCoupledVar("rhoEA", "total energy multiplied by area");
  params.addRequiredCoupledVar("vel", "velocity");
  params.addRequiredCoupledVar("H", "Specific total enthalpy");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<MaterialPropertyName>("pressure", "Pressure");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction material property");

  return params;
}

OneDEnergyFlux::OneDEnergyFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _rhouA(coupledValue("rhouA")),
    _area(coupledValue("area")),
    _vel(coupledValue("vel")),
    _enthalpy(coupledValue("H")),
    _pressure(getMaterialProperty<Real>("pressure")),
    _dp_darhoA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhoEA")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _has_beta(isCoupled("beta")),
    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _dp_dbeta(_has_beta ? &getMaterialPropertyDerivativeRelap<Real>("pressure", "beta") : nullptr),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(_has_beta ? &getMaterialPropertyDerivativeRelap<Real>("alpha", "beta") : nullptr)
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
  Real A33 = _vel[_qp] * (1. + _alpha[_qp] * _dp_darhoEA[_qp] * _area[_qp]);
  return -A33 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDEnergyFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    Real A31 = _vel[_qp] * (_alpha[_qp] * _dp_darhoA[_qp] * _area[_qp] - _enthalpy[_qp]);
    return -A31 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _rhouA_var_number)
  {
    Real A32 = _vel[_qp] * _alpha[_qp] * _dp_darhouA[_qp] * _area[_qp] + _enthalpy[_qp];
    return -A32 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _beta_var_number)
  {
    return -(_vel[_qp] *
             (_pressure[_qp] * (*_dalpha_dbeta)[_qp] + _alpha[_qp] * (*_dp_dbeta)[_qp])) *
           _area[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
