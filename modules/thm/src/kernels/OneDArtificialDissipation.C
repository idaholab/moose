#include "OneDArtificialDissipation.h"

registerMooseObject("THMApp", OneDArtificialDissipation);

template <>
InputParameters
validParams<OneDArtificialDissipation>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("U", "Variable to smooth");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  params.addRequiredParam<MaterialPropertyName>(
      "coef_name", "The dissipation coeffcient material property name");

  return params;
}

OneDArtificialDissipation::OneDArtificialDissipation(const InputParameters & parameters)
  : Kernel(parameters),
    _area(coupledValue("A")),
    _U(coupledValue("U")),
    _grad_U(coupledGradient("U")),
    _alpha(coupledValue("alpha")),
    _grad_alpha(isCoupled("alpha") ? coupledGradient("alpha") : _grad_zero),
    _coef(getMaterialProperty<Real>("coef_name"))
{
}

Real
OneDArtificialDissipation::computeQpResidual()
{
  // U = { rho, rhou, rhoE }
  // coef * A * \grad (\alpha U) * phi = coef * A * (\alpha \grad U + U \grad \alpha) * phi
  return _coef[_qp] * _area[_qp] * (_alpha[_qp] * _grad_U[_qp] + _grad_alpha[_qp] * _U[_qp]) *
         _grad_test[_i][_qp];
}

Real
OneDArtificialDissipation::computeQpJacobian()
{
  return _coef[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
