#include "OneD3EqnEnergyFriction.h"

registerMooseObject("THMApp", OneD3EqnEnergyFriction);

template <>
InputParameters
validParams<OneD3EqnEnergyFriction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredCoupledVar("arhoA", "Solution variable alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "Solution variable alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "Solution variable alpha*rho*E*A");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");
  params.addRequiredParam<MaterialPropertyName>("f_D",
                                                "Darcy friction factor coefficient property");
  params.addClassDescription("Computes energy dissipation caused by wall friction in 1-phase flow");
  return params;
}

OneD3EqnEnergyFriction::OneD3EqnEnergyFriction(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),
    _D_h(getMaterialProperty<Real>("D_h")),
    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),
    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),
    _f_D(getMaterialProperty<Real>("f_D")),
    _df_D_darhoA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhoA")),
    _df_D_darhouA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhouA")),
    _df_D_darhoEA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhoEA")),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

Real
OneD3EqnEnergyFriction::computeQpResidual()
{
  return -0.5 * _f_D[_qp] * _rho[_qp] * _vel[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] /
         _D_h[_qp] * _test[_i][_qp];
}

Real
OneD3EqnEnergyFriction::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneD3EqnEnergyFriction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    const Real vel3 = _vel[_qp] * _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel3_darhoA = 3 * _vel[_qp] * std::abs(_vel[_qp]) * _dvel_darhoA[_qp];
    return -(_df_D_darhoA[_qp] * _rho[_qp] * vel3 + _f_D[_qp] * _drho_darhoA[_qp] * vel3 +
             _f_D[_qp] * _rho[_qp] * dvel3_darhoA) *
           0.5 * _A[_qp] / _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    const Real vel3 = _vel[_qp] * _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel3_darhouA = 3 * _vel[_qp] * std::abs(_vel[_qp]) * _dvel_darhouA[_qp];
    return -(_df_D_darhouA[_qp] * vel3 + _f_D[_qp] * dvel3_darhouA) * 0.5 * _rho[_qp] * _A[_qp] /
           _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return -(_df_D_darhoEA[_qp]) * 0.5 * _rho[_qp] * _vel[_qp] * _vel[_qp] * std::abs(_vel[_qp]) *
           _A[_qp] / _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
