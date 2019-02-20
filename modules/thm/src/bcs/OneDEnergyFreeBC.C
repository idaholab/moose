#include "OneDEnergyFreeBC.h"

registerMooseObject("THMApp", OneDEnergyFreeBC);

template <>
InputParameters
validParams<OneDEnergyFreeBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");
  params.addRequiredCoupledVar("H", "Specific total enthalpy");
  params.addRequiredCoupledVar("vel", "x-component of velocity");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addParam<bool>("is_liquid", true, "True for liquid, false for vapor");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure property name");
  return params;
}

OneDEnergyFreeBC::OneDEnergyFreeBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA")),
    _arhouA(coupledValue("arhouA")),
    _enthalpy(coupledValue("H")),
    _vel(coupledValue("vel")),
    _area(coupledValue("A")),
    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),
    _has_beta(isCoupled("beta")),
    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _dp_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("p", "beta") : nullptr),
    _alpha(coupledValue("alpha"))
{
}

Real
OneDEnergyFreeBC::computeQpResidual()
{
  Real F3 = _arhouA[_qp] * _enthalpy[_qp];
  return F3 * _normal * _test[_i][_qp];
}

Real
OneDEnergyFreeBC::computeQpJacobian()
{
  return _vel[_qp] * (1 + _alpha[_qp] * _dp_darhoEA[_qp] * _area[_qp]) * _normal * _phi[_j][_qp] *
         _test[_i][_qp];
}

Real
OneDEnergyFreeBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _arhoA_var_number)
    return _vel[_qp] * (_alpha[_qp] * _dp_darhoA[_qp] * _area[_qp] - _enthalpy[_qp]) * _normal *
           _phi[_j][_qp] * _test[_i][_qp];

  else if (jvar == _arhouA_var_number)
    return (_enthalpy[_qp] + _vel[_qp] * _alpha[_qp] * _area[_qp] * _dp_darhouA[_qp]) * _normal *
           _phi[_j][_qp] * _test[_i][_qp];

  else if (jvar == _beta_var_number)
    return _vel[_qp] * (_sign * _p[_qp] + _alpha[_qp] * (*_dp_dbeta)[_qp]) * _area[_qp] * _normal *
           _phi[_j][_qp] * _test[_i][_qp];

  else
    return 0.;
}
