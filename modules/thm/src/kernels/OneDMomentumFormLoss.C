#include "OneDMomentumFormLoss.h"
#include "Function.h"

template <>
InputParameters
validParams<OneDMomentumFormLoss>()
{
  InputParameters params = validParams<Kernel>();

  params.addCoupledVar("beta", "Solution variable beta");
  params.addRequiredCoupledVar("arhoA", "Conserved density");
  params.addRequiredCoupledVar("arhouA", "Conserved momentum");

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");

  params.addRequiredParam<FunctionName>("K_prime",
                                        "Form loss coefficient per unit length function");

  return params;
}

OneDMomentumFormLoss::OneDMomentumFormLoss(const InputParameters & parameters)
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

    _K_prime(getFunction("K_prime")),

    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA"))
{
}

OneDMomentumFormLoss::~OneDMomentumFormLoss() {}

Real
OneDMomentumFormLoss::computeQpResidual()
{
  return _K_prime.value(_t, _q_point[_qp]) * 0.5 * _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) *
         _alpha[_qp] * _A[_qp] * _test[_i][_qp];
}

Real
OneDMomentumFormLoss::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneDMomentumFormLoss::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
  {
    const Real drhou2_dbeta = (*_drho_dbeta)[_qp] * _vel[_qp] * std::abs(_vel[_qp]);

    return _K_prime.value(_t, _q_point[_qp]) * 0.5 *
           (drhou2_dbeta * _alpha[_qp] +
            _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * (*_dalpha_dbeta)[_qp]) *
           _A[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  if (jvar == _arhoA_var_number)
  {
    const Real drhou2_darhoA = _drho_darhoA[_qp] * _vel[_qp] * std::abs(_vel[_qp]) +
                               2.0 * _rho[_qp] * std::abs(_vel[_qp]) * _dvel_darhoA[_qp];

    return _K_prime.value(_t, _q_point[_qp]) * 0.5 * drhou2_darhoA * _alpha[_qp] * _A[_qp] *
           _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    const Real drhou2_darhouA = 2.0 * _rho[_qp] * std::abs(_vel[_qp]) * _dvel_darhouA[_qp];

    return _K_prime.value(_t, _q_point[_qp]) * 0.5 * drhou2_darhouA * _alpha[_qp] * _A[_qp] *
           _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
