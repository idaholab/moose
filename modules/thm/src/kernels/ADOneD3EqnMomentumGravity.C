#include "ADOneD3EqnMomentumGravity.h"

registerMooseObject("THMApp", ADOneD3EqnMomentumGravity);

InputParameters
ADOneD3EqnMomentumGravity::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");
  params.addClassDescription("Computes gravity term for the momentum equation for 1-phase flow");
  return params;
}

ADOneD3EqnMomentumGravity::ADOneD3EqnMomentumGravity(const InputParameters & parameters)
  : ADKernel(parameters),
    _A(adCoupledValue("A")),
    _rho(getADMaterialProperty<Real>("rho")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector"))
{
}

ADReal
ADOneD3EqnMomentumGravity::computeQpResidual()
{
  return -_rho[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _test[_i][_qp];
}
