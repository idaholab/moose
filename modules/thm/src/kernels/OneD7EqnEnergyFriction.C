#include "OneD7EqnEnergyFriction.h"

registerMooseObject("THMApp", OneD7EqnEnergyFriction);

InputParameters
OneD7EqnEnergyFriction::validParams()
{
  InputParameters params = OneD3EqnEnergyFriction::validParams();
  params.addRequiredCoupledVar("beta", "Solution variable beta");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction property");
  params.addRequiredParam<MaterialPropertyName>("2phase_multiplier", "2-phase multiplier property");
  params.addClassDescription("Computes energy dissipation caused by wall friction in 2-phase flow");
  return params;
}

OneD7EqnEnergyFriction::OneD7EqnEnergyFriction(const InputParameters & parameters)
  : OneD3EqnEnergyFriction(parameters),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")),
    _drho_dbeta(getMaterialPropertyDerivativeTHM<Real>("rho", "beta")),
    _df_D_dbeta(getMaterialPropertyDerivativeTHM<Real>("f_D", "beta")),
    _mult(getMaterialProperty<Real>("2phase_multiplier")),
    _dmult_dbeta(getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "beta")),
    _dmult_darhoA(getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "arhoA")),
    _dmult_darhouA(getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "arhouA")),
    _dmult_darhoEA(getMaterialPropertyDerivativeTHM<Real>("2phase_multiplier", "arhoEA")),
    _beta_var_number(coupled("beta"))
{
}

Real
OneD7EqnEnergyFriction::computeQpResidual()
{
  return _mult[_qp] * _alpha[_qp] * OneD3EqnEnergyFriction::computeQpResidual();
}

Real
OneD7EqnEnergyFriction::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneD7EqnEnergyFriction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
  {
    return -(_dmult_dbeta[_qp] * _f_D[_qp] * _alpha[_qp] * _rho[_qp] +
             _mult[_qp] * _df_D_dbeta[_qp] * _alpha[_qp] * _rho[_qp] +
             _mult[_qp] * _f_D[_qp] * _dalpha_dbeta[_qp] * _rho[_qp] +
             _mult[_qp] * _f_D[_qp] * _alpha[_qp] * _drho_dbeta[_qp]) *
           0.5 * _vel[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] / _D_h[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    return _alpha[_qp] *
           (_dmult_darhoA[_qp] * OneD3EqnEnergyFriction::computeQpResidual() * _phi[_j][_qp] +
            _mult[_qp] * OneD3EqnEnergyFriction::computeQpOffDiagJacobian(jvar));
  }
  else if (jvar == _arhouA_var_number)
  {
    return _alpha[_qp] *
           (_dmult_darhouA[_qp] * OneD3EqnEnergyFriction::computeQpResidual() * _phi[_j][_qp] +
            _mult[_qp] * OneD3EqnEnergyFriction::computeQpOffDiagJacobian(jvar));
  }
  else if (jvar == _arhoEA_var_number)
  {
    return _alpha[_qp] *
           (_dmult_darhoEA[_qp] * OneD3EqnEnergyFriction::computeQpResidual() * _phi[_j][_qp] +
            _mult[_qp] * OneD3EqnEnergyFriction::computeQpOffDiagJacobian(jvar));
  }
  else
    return 0;
}
