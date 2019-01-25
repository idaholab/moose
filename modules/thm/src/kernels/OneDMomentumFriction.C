#include "OneDMomentumFriction.h"

registerMooseObject("THMApp", OneDMomentumFriction);

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
  params.addRequiredParam<MaterialPropertyName>("2phase_multiplier", "2-phase multiplier property");
  return params;
}

OneDMomentumFriction::OneDMomentumFriction(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _Cw(getMaterialProperty<Real>("Cw")),
    _dCw_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("Cw", "beta") : nullptr),
    _dCw_darhoA(getMaterialPropertyDerivativeTHM<Real>("Cw", "arhoA")),
    _dCw_darhouA(getMaterialPropertyDerivativeTHM<Real>("Cw", "arhouA")),
    _dCw_darhoEA(getMaterialPropertyDerivativeTHM<Real>("Cw", "arhoEA")),

    _mult(getMaterialProperty<Real>("2phase_multiplier")),
    _dmult_dbeta(isCoupled("beta")
                     ? &getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "beta")
                     : nullptr),
    _dmult_darhoA(getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "arhoA")),
    _dmult_darhouA(getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "arhouA")),
    _dmult_darhoEA(getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "arhoEA")),

    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

Real
OneDMomentumFriction::computeQpResidual()
{
  return _mult[_qp] * _Cw[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] * _test[_i][_qp];
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
    return ((*_dmult_dbeta)[_qp] * _Cw[_qp] + _mult[_qp] * (*_dCw_dbeta)[_qp]) * _vel[_qp] *
           std::abs(_vel[_qp]) * _A[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    const Real vel2 = _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel2_darhoA = 2 * std::abs(_vel[_qp]) * _dvel_darhoA[_qp];
    return (_dmult_darhoA[_qp] * _Cw[_qp] * vel2 + _mult[_qp] * _dCw_darhoA[_qp] * vel2 +
            _mult[_qp] * _Cw[_qp] * dvel2_darhoA) *
           _A[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    const Real vel2 = _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel2_darhouA = 2 * std::abs(_vel[_qp]) * _dvel_darhouA[_qp];
    return (_dmult_darhouA[_qp] * _Cw[_qp] * vel2 + _mult[_qp] * _dCw_darhouA[_qp] * vel2 +
            _mult[_qp] * _Cw[_qp] * dvel2_darhouA) *
           _A[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return (_dmult_darhoEA[_qp] * _Cw[_qp] + _mult[_qp] * _dCw_darhoEA[_qp]) * _vel[_qp] *
           std::abs(_vel[_qp]) * _A[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
