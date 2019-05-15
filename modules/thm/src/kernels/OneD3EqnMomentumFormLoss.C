#include "OneD3EqnMomentumFormLoss.h"
#include "Function.h"

registerMooseObject("THMApp", OneD3EqnMomentumFormLoss);

template <>
InputParameters
validParams<OneD3EqnMomentumFormLoss>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("arhoA", "Solution variable alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "Solution variable alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "Solution variable alpha*rho*E*A");

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");

  return params;
}

OneD3EqnMomentumFormLoss::OneD3EqnMomentumFormLoss(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),

    _A(coupledValue("A")),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _K_prime(getMaterialProperty<Real>("K_prime")),

    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

Real
OneD3EqnMomentumFormLoss::computeQpResidual()
{
  return _K_prime[_qp] * 0.5 * _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) *
         _A[_qp] * _test[_i][_qp];
}

Real
OneD3EqnMomentumFormLoss::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneD3EqnMomentumFormLoss::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    const Real drhou2_darhoA = _drho_darhoA[_qp] * _vel[_qp] * std::abs(_vel[_qp]) +
                               2.0 * _rho[_qp] * std::abs(_vel[_qp]) * _dvel_darhoA[_qp];

    return _K_prime[_qp] * 0.5 * drhou2_darhoA * _A[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    const Real drhou2_darhouA = 2.0 * _rho[_qp] * std::abs(_vel[_qp]) * _dvel_darhouA[_qp];

    return _K_prime[_qp] * 0.5 * drhou2_darhouA * _A[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
    return 0.;
  else
    return 0;
}
