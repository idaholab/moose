#include "ADOneDEnergyWallHeating.h"

registerMooseObject("THMApp", ADOneDEnergyWallHeating);

InputParameters
ADOneDEnergyWallHeating::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");
  params.addCoupledVar("T_wall", 0, "Wall temperature (const)");
  params.addRequiredParam<MaterialPropertyName>("Hw",
                                                "Convective heat transfer coefficient, W/m^2-K");
  params.addRequiredParam<MaterialPropertyName>("T", "Temperature material property");

  return params;
}

ADOneDEnergyWallHeating::ADOneDEnergyWallHeating(const InputParameters & parameters)
  : ADKernel(parameters),
    _temperature(getADMaterialProperty<Real>("T")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _T_wall(adCoupledValue("T_wall")),
    _P_hf(adCoupledValue("P_hf"))
{
}

ADReal
ADOneDEnergyWallHeating::computeQpResidual()
{
  return _Hw[_qp] * _P_hf[_qp] * (_temperature[_qp] - _T_wall[_qp]) * _test[_i][_qp];
}
