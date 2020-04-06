#include "OneD3EqnMassFlux.h"

registerMooseObject("THMApp", OneD3EqnMassFlux);

InputParameters
OneD3EqnMassFlux::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");

  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");

  params.addClassDescription("Mass flux for 1-phase flow");

  return params;
}

OneD3EqnMassFlux::OneD3EqnMassFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _arhouA_var_number(coupled("arhouA"))
{
}

Real
OneD3EqnMassFlux::computeQpResidual()
{
  return -_rho[_qp] * _vel[_qp] * _dir[_qp] * _A[_qp] * _grad_test[_i][_qp];
}

Real
OneD3EqnMassFlux::computeQpJacobian()
{
  return -(_drho_darhoA[_qp] * _vel[_qp] + _rho[_qp] * _dvel_darhoA[_qp]) * _dir[_qp] * _A[_qp] *
         _phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
OneD3EqnMassFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhouA_var_number)
    return -_rho[_qp] * _dvel_darhouA[_qp] * _dir[_qp] * _A[_qp] * _phi[_j][_qp] *
           _grad_test[_i][_qp];
  else
    return 0.;
}
