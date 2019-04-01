#include "OneDMassFlux.h"

registerMooseObject("THMApp", OneDMassFlux);

template <>
InputParameters
validParams<OneDMassFlux>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("beta", "Volume fraction mapper");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");

  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");

  return params;
}

OneDMassFlux::OneDMassFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("rho", "beta")
                                  : nullptr),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhouA_var_number(coupled("arhouA"))
{
}

Real
OneDMassFlux::computeQpResidual()
{
  return -_alpha[_qp] * _rho[_qp] * _vel[_qp] * _dir[_qp] * _A[_qp] * _grad_test[_i][_qp];
}

Real
OneDMassFlux::computeQpJacobian()
{
  return -_alpha[_qp] * (_drho_darhoA[_qp] * _vel[_qp] + _rho[_qp] * _dvel_darhoA[_qp]) *
         _dir[_qp] * _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
OneDMassFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
    return -((*_dalpha_dbeta)[_qp] * _rho[_qp] + _alpha[_qp] * (*_drho_dbeta)[_qp]) * _vel[_qp] *
           _dir[_qp] * _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  else if (jvar == _arhouA_var_number)
    return -_alpha[_qp] * _rho[_qp] * _dvel_darhouA[_qp] * _dir[_qp] * _A[_qp] * _phi[_j][_qp] *
           _grad_test[_i][_qp];
  else
    return 0.;
}
