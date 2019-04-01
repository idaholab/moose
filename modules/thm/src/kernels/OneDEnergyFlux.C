#include "OneDEnergyFlux.h"

registerMooseObject("THMApp", OneDEnergyFlux);

template <>
InputParameters
validParams<OneDEnergyFlux>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("A", "Cross-sectional area");

  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");

  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density material property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity material property");
  params.addRequiredParam<MaterialPropertyName>("e", "Specific internal energy material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure material property");

  return params;
}

OneDEnergyFlux::OneDEnergyFlux(const InputParameters & parameters)
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

    _e(getMaterialProperty<Real>("e")),
    _de_darhoA(getMaterialPropertyDerivativeTHM<Real>("e", "arhoA")),
    _de_darhouA(getMaterialPropertyDerivativeTHM<Real>("e", "arhouA")),
    _de_darhoEA(getMaterialPropertyDerivativeTHM<Real>("e", "arhoEA")),

    _p(getMaterialProperty<Real>("p")),
    _dp_dbeta(_has_beta ? &getMaterialPropertyDerivativeTHM<Real>("p", "beta") : nullptr),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),

    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA"))
{
}

Real
OneDEnergyFlux::computeQpResidual()
{
  return -_alpha[_qp] * _vel[_qp] * _dir[_qp] *
         (_rho[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) + _p[_qp]) * _A[_qp] *
         _grad_test[_i][_qp];
}

Real
OneDEnergyFlux::computeQpJacobian()
{
  return -_alpha[_qp] * _vel[_qp] * _dir[_qp] * (_rho[_qp] * _de_darhoEA[_qp] + _dp_darhoEA[_qp]) *
         _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
OneDEnergyFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -_alpha[_qp] *
           (_dvel_darhoA[_qp] * (_rho[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) + _p[_qp]) +
            _vel[_qp] *
                (_drho_darhoA[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) +
                 _rho[_qp] * (_de_darhoA[_qp] + _vel[_qp] * _dvel_darhoA[_qp]) + _dp_darhoA[_qp])) *
           _dir[_qp] * _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    return -_alpha[_qp] *
           (_dvel_darhouA[_qp] * (_rho[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) + _p[_qp]) +
            _vel[_qp] * (_rho[_qp] * (_de_darhouA[_qp] + _vel[_qp] * _dvel_darhouA[_qp]) +
                         _dp_darhouA[_qp])) *
           _dir[_qp] * _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else if (jvar == _beta_var_number)
  {
    return -((*_dalpha_dbeta)[_qp] * _vel[_qp] *
                 (_rho[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) + _p[_qp]) +
             _alpha[_qp] * _vel[_qp] *
                 ((*_drho_dbeta)[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) +
                  (*_dp_dbeta)[_qp])) *
           _dir[_qp] * _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else
    return 0.;
}
