#include "OneDMassFlux.h"

registerMooseObject("RELAP7App", OneDMassFlux);

template <>
InputParameters
validParams<OneDMassFlux>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("beta", "Volume fraction mapper");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");

  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");

  return params;
}

OneDMassFlux::OneDMassFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _A(coupledValue("A")),

    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("alpha", "beta")
                                    : nullptr),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("rho", "beta")
                                  : nullptr),
    _drho_darhoA(getMaterialPropertyDerivativeRelap<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeRelap<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeRelap<Real>("vel", "arhouA")),

    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhouA_var_number(coupled("arhouA"))
{
}

Real
OneDMassFlux::computeQpResidual()
{
  return -_alpha[_qp] * _rho[_qp] * _vel[_qp] * _A[_qp] * _grad_test[_i][_qp](0);
}

Real
OneDMassFlux::computeQpJacobian()
{
  return -_alpha[_qp] * (_drho_darhoA[_qp] * _vel[_qp] + _rho[_qp] * _dvel_darhoA[_qp]) * _A[_qp] *
         _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDMassFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
    return -((*_dalpha_dbeta)[_qp] * _rho[_qp] + _alpha[_qp] * (*_drho_dbeta)[_qp]) * _vel[_qp] *
           _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  else if (jvar == _arhouA_var_number)
    return -_alpha[_qp] * _rho[_qp] * _dvel_darhouA[_qp] * _A[_qp] * _phi[_j][_qp] *
           _grad_test[_i][_qp](0);
  else
    return 0.;
}
