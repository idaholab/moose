#include "ElementHeatFluxPostprocessor.h"

registerMooseObject("THMApp", ElementHeatFluxPostprocessor);

template <>
InputParameters
validParams<ElementHeatFluxPostprocessor>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<MaterialPropertyName>("T_wall", "Wall temperature");
  params.addRequiredCoupledVar("Tfluid", "Temperature of the fluid on the slave side");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Wall heat transfer coefficient");
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");

  return params;
}

ElementHeatFluxPostprocessor::ElementHeatFluxPostprocessor(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _T_wall(getMaterialProperty<Real>("T_wall")),
    _Tfluid(coupledValue("Tfluid")),
    _Hw(getMaterialProperty<Real>("Hw")),
    _P_hf(coupledValue("P_hf"))
{
}

Real
ElementHeatFluxPostprocessor::computeQpIntegral()
{
  return -_Hw[_qp] * _P_hf[_qp] * (_Tfluid[_qp] - _T_wall[_qp]);
}
