#include "OneDMomentumFlux.h"

template <>
InputParameters
validParams<OneDMomentumFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("alpha", 1.0, "Volume fraction");
  params.addCoupledVar("beta", 0, "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addRequiredCoupledVar("rhouA", "momentum multiplied by area");
  params.addRequiredCoupledVar("rhoEA", "total energy multiplied by area");
  params.addRequiredCoupledVar("vel", "velocity");
  params.addRequiredCoupledVar("area", "cross-sectional area");
  params.addParam<bool>("is_liquid", true, "True for liquid, false for vapor");
  params.addRequiredParam<MaterialPropertyName>("pressure", "Pressure");
  return params;
}

OneDMomentumFlux::OneDMomentumFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _alpha(coupledValue("alpha")),
    _vel(coupledValue("vel")),
    _pressure(getMaterialProperty<Real>("pressure")),
    _dp_darhoA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhoEA")),
    _area(coupledValue("area")),
    _rhoA_var_number(coupled("rhoA")),
    _rhoEA_var_number(coupled("rhoEA")),
    _has_beta(isCoupled("beta")),
    _beta(coupledValue("beta")),
    _beta_var_number(_has_beta ? coupled("beta") : libMesh::invalid_uint),
    _dp_dbeta(_has_beta ? &getMaterialPropertyDerivativeRelap<Real>("pressure", "beta") : NULL),
    _daL_dbeta(_has_beta ? &getMaterialProperty<Real>("daL_dbeta") : NULL)
{
}

OneDMomentumFlux::~OneDMomentumFlux() {}

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
    return -(_sign * _pressure[_qp] * (*_daL_dbeta)[_qp] + _alpha[_qp] * (*_dp_dbeta)[_qp]) *
           _area[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
