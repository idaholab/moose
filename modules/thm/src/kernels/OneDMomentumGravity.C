#include "OneDMomentumGravity.h"

registerMooseObject("THMApp", OneDMomentumGravity);

template <>
InputParameters
validParams<OneDMomentumGravity>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("A", "Cross-sectional area");

  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");

  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "The direction of the pipe material property");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");

  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");

  return params;
}

OneDMomentumGravity::OneDMomentumGravity(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _has_beta(isCoupled("beta")),

    _A(coupledValue("A")),

    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta") : nullptr),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("rho", "beta") : nullptr),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector")),

    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA"))
{
}

Real
OneDMomentumGravity::computeQpResidual()
{
  return -_alpha[_qp] * _rho[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _test[_i][_qp];
}

Real
OneDMomentumGravity::computeQpJacobian()
{
  return 0;
}

Real
OneDMomentumGravity::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
  {
    return -((*_dalpha_dbeta)[_qp] * _rho[_qp] + _alpha[_qp] * (*_drho_dbeta)[_qp]) * _A[_qp] *
           _gravity_vector * _dir[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    return -_alpha[_qp] * _drho_darhoA[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] *
           _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
