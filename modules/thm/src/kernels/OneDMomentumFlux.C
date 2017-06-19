#include "OneDMomentumFlux.h"

template <>
InputParameters
validParams<OneDMomentumFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("beta", 0, "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("rhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("rhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "alpha*rho*E*A");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("A", "cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure");
  return params;
}

OneDMomentumFlux::OneDMomentumFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("alpha", "beta")
                                    : nullptr),
    _vel(coupledValue("vel")),
    _pressure(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeRelap<Real>("p", "rhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeRelap<Real>("p", "rhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeRelap<Real>("p", "rhoEA")),
    _area(coupledValue("A")),
    _rhoA_var_number(coupled("rhoA")),
    _rhoEA_var_number(coupled("rhoEA")),
    _beta(coupledValue("beta")),
    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _dp_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("p", "beta") : nullptr)
{
}

Real
OneDMomentumFlux::computeQpResidual()
{
  Real F2 = _u[_qp] * _vel[_qp] + _alpha[_qp] * _pressure[_qp] * _area[_qp];
  // The contribution due to the convective flux.  Negative sign on
  // the F2 term comes from integration by parts.
  return -F2 * _grad_test[_i][_qp](0);
}

Real
OneDMomentumFlux::computeQpJacobian()
{
  // (2,2) entry of flux Jacobian is the same as the constant area case, p_1 + 2*u
  Real A22 = 2. * _vel[_qp] + _alpha[_qp] * _dp_darhouA[_qp] * _area[_qp];

  // Negative sign comes from integration by parts
  return -A22 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDMomentumFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    // (2,1) entry of flux Jacobian is the same as the constant area case, p_0 - u^2
    Real A21 = _alpha[_qp] * _dp_darhoA[_qp] * _area[_qp] - _vel[_qp] * _vel[_qp];

    // The contribution from the convective flux term.  Negative sign comes from integration by
    // parts.
    return -A21 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _rhoEA_var_number)
  {
    // (2,3) entry of flux Jacobian is the same as the constant area case, p_2
    Real A23 = _alpha[_qp] * _dp_darhoEA[_qp] * _area[_qp];

    // Contribution due to convective flux.  Negative sign comes from integration by parts.
    return -A23 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _beta_var_number)
  {
    return -(_pressure[_qp] * (*_dalpha_dbeta)[_qp] + _alpha[_qp] * (*_dp_dbeta)[_qp]) *
           _area[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
