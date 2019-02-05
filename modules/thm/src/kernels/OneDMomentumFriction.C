#include "OneDMomentumFriction.h"

registerMooseObject("THMApp", OneDMomentumFriction);

template <>
InputParameters
validParams<OneDMomentumFriction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("D_h", "Hydraulic diameter");
  params.addCoupledVar("beta", "Solution variable beta");
  params.addRequiredCoupledVar("arhoA", "Solution variable alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "Solution variable alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "Solution variable alpha*rho*E*A");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");
  params.addRequiredParam<MaterialPropertyName>("f_D",
                                                "Darcy friction factor coefficient property");
  params.addRequiredParam<MaterialPropertyName>("2phase_multiplier", "2-phase multiplier property");
  return params;
}

OneDMomentumFriction::OneDMomentumFriction(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),
    _D_h(coupledValue("D_h")),

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

    _f_D(getMaterialProperty<Real>("f_D")),
    _df_D_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("f_D", "beta")
                                  : nullptr),
    _df_D_darhoA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhoA")),
    _df_D_darhouA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhouA")),
    _df_D_darhoEA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhoEA")),

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
  return _mult[_qp] * 0.5 * _f_D[_qp] * _alpha[_qp] * _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) *
         _A[_qp] / _D_h[_qp] * _test[_i][_qp];
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
    return ((*_dmult_dbeta)[_qp] * _f_D[_qp] * _alpha[_qp] * _rho[_qp] +
            _mult[_qp] * (*_df_D_dbeta)[_qp] * _alpha[_qp] * _rho[_qp] +
            _mult[_qp] * _f_D[_qp] * (*_dalpha_dbeta)[_qp] * _rho[_qp] +
            _mult[_qp] * _f_D[_qp] * _alpha[_qp] * (*_drho_dbeta)[_qp]) *
           0.5 * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] / _D_h[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    const Real vel2 = _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel2_darhoA = 2 * std::abs(_vel[_qp]) * _dvel_darhoA[_qp];
    return (_dmult_darhoA[_qp] * _f_D[_qp] * _rho[_qp] * vel2 +
            _mult[_qp] * _df_D_darhoA[_qp] * _rho[_qp] * vel2 +
            _mult[_qp] * _f_D[_qp] * _drho_darhoA[_qp] * vel2 +
            _mult[_qp] * _f_D[_qp] * _rho[_qp] * dvel2_darhoA) *
           0.5 * _alpha[_qp] * _A[_qp] / _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    const Real vel2 = _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel2_darhouA = 2 * std::abs(_vel[_qp]) * _dvel_darhouA[_qp];
    return (_dmult_darhouA[_qp] * _f_D[_qp] * vel2 + _mult[_qp] * _df_D_darhouA[_qp] * vel2 +
            _mult[_qp] * _f_D[_qp] * dvel2_darhouA) *
           0.5 * _alpha[_qp] * _rho[_qp] * _A[_qp] / _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return (_dmult_darhoEA[_qp] * _f_D[_qp] + _mult[_qp] * _df_D_darhoEA[_qp]) * 0.5 * _alpha[_qp] *
           _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] / _D_h[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else
    return 0;
}
