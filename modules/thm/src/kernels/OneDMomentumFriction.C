#include "OneDMomentumFriction.h"

template <>
InputParameters
validParams<OneDMomentumFriction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("beta", "Solution variable beta");
  params.addRequiredCoupledVar("arhoA", "Solution variable alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "Solution variable alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "Solution variable alpha*rho*E*A");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");
  params.addRequiredParam<MaterialPropertyName>("Cw", "Wall drag coefficient property");
  return params;
}

OneDMomentumFriction::OneDMomentumFriction(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _A(coupledValue("A")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeRelap<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeRelap<Real>("vel", "arhouA")),

    _Cw(getMaterialProperty<Real>("Cw")),
    _dCw_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("Cw", "beta")
                                 : nullptr),
    _dCw_drhoA(getMaterialPropertyDerivativeRelap<Real>("Cw", "arhoA")),
    _dCw_drhouA(getMaterialPropertyDerivativeRelap<Real>("Cw", "arhouA")),
    _dCw_drhoEA(getMaterialPropertyDerivativeRelap<Real>("Cw", "arhoEA")),

    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

OneDMomentumFriction::~OneDMomentumFriction() {}

Real
OneDMomentumFriction::computeQpResidual()
{
  return _Cw[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] * _test[_i][_qp];
}

Real
OneDMomentumFriction::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneDMomentumFriction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
  {
    return (*_dCw_dbeta)[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    Real dCwu2_darhoA = _dCw_drhoA[_qp] * _vel[_qp] * std::abs(_vel[_qp]) +
                        _Cw[_qp] * 2.0 * std::abs(_vel[_qp]) * _dvel_darhoA[_qp];
    return dCwu2_darhoA * _A[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    Real dCwu2_darhouA = _dCw_drhouA[_qp] * _vel[_qp] * std::abs(_vel[_qp]) +
                         _Cw[_qp] * 2.0 * std::abs(_vel[_qp]) * _dvel_darhouA[_qp];
    return dCwu2_darhouA * _A[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return _dCw_drhoEA[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else
    return 0;
}
