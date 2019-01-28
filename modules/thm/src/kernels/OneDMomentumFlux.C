#include "OneDMomentumFlux.h"

registerMooseObject("THMApp", OneDMomentumFlux);

template <>
InputParameters
validParams<OneDMomentumFlux>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("A", "Cross-sectional area");

  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "The direction of the pipe material property");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density material property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure material property");

  return params;
}

OneDMomentumFlux::OneDMomentumFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),

    _has_beta(isCoupled("beta")),

    _A(coupledValue("A")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta") : nullptr),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("rho", "beta") : nullptr),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _p(getMaterialProperty<Real>("p")),
    _dp_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("p", "beta") : nullptr),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),

    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

Real
OneDMomentumFlux::computeQpResidual()
{
  return -_alpha[_qp] * (_rho[_qp] * _vel[_qp] * _vel[_qp] + _p[_qp]) * _A[_qp] * _dir[_qp] *
         _grad_test[_i][_qp];
}

Real
OneDMomentumFlux::computeQpJacobian()
{
  return -_alpha[_qp] * (_rho[_qp] * 2.0 * _vel[_qp] * _dvel_darhouA[_qp] + _dp_darhouA[_qp]) *
         _A[_qp] * _dir[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
OneDMomentumFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -_alpha[_qp] *
           (_drho_darhoA[_qp] * _vel[_qp] * _vel[_qp] +
            _rho[_qp] * 2.0 * _vel[_qp] * _dvel_darhoA[_qp] + _dp_darhoA[_qp]) *
           _A[_qp] * _dir[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return -_alpha[_qp] * _dp_darhoEA[_qp] * _A[_qp] * _dir[_qp] * _phi[_j][_qp] *
           _grad_test[_i][_qp];
  }
  else if (jvar == _beta_var_number)
  {
    return -((*_dalpha_dbeta)[_qp] * (_rho[_qp] * _vel[_qp] * _vel[_qp] + _p[_qp]) +
             _alpha[_qp] * ((*_drho_dbeta)[_qp] * _vel[_qp] * _vel[_qp] + (*_dp_dbeta)[_qp])) *
           _A[_qp] * _dir[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else
    return 0.;
}
