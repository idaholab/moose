#include "ADOneD3EqnEnergyGravity.h"

registerMooseObject("THMApp", ADOneD3EqnEnergyGravity);

InputParameters
ADOneD3EqnEnergyGravity::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");
  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");
  params.addClassDescription("Computes gravity term for the energy equation in 1-phase flow");
  return params;
}

ADOneD3EqnEnergyGravity::ADOneD3EqnEnergyGravity(const InputParameters & parameters)
  : ADKernel(parameters),
    _A(adCoupledValue("A")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector"))
{
}

ADReal
ADOneD3EqnEnergyGravity::computeQpResidual()
{
  return -_rho[_qp] * _vel[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _test[_i][_qp];
}
