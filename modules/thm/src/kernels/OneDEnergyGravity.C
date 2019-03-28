#include "OneDEnergyGravity.h"

registerMooseObject("THMApp", OneDEnergyGravity);

template <>
InputParameters
validParams<OneDEnergyGravity>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("A", "Cross-sectional area");

  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");

  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");

  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");

  return params;
}

OneDEnergyGravity::OneDEnergyGravity(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _has_beta(isCoupled("beta")),

    _A(coupledValue("A")),

    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta") : nullptr),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("rho", "beta") : nullptr),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector")),

    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA"))
{
}

Real
OneDEnergyGravity::computeQpResidual()
{
  return -_alpha[_qp] * _rho[_qp] * _vel[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] *
         _test[_i][_qp];
}

Real
OneDEnergyGravity::computeQpJacobian()
{
  return 0;
}

Real
OneDEnergyGravity::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
  {
    return -((*_dalpha_dbeta)[_qp] * _rho[_qp] + _alpha[_qp] * (*_drho_dbeta)[_qp]) * _vel[_qp] *
           _A[_qp] * _gravity_vector * _dir[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    return -_alpha[_qp] * (_drho_darhoA[_qp] * _vel[_qp] + _rho[_qp] * _dvel_darhoA[_qp]) *
           _A[_qp] * _gravity_vector * _dir[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    return -_alpha[_qp] * _rho[_qp] * _dvel_darhouA[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] *
           _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
