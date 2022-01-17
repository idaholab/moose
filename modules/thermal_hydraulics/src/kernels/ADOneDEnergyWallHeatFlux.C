#include "ADOneDEnergyWallHeatFlux.h"

registerMooseObject("ThermalHydraulicsApp", ADOneDEnergyWallHeatFlux);

InputParameters
ADOneDEnergyWallHeatFlux::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("q_wall", "Wall heat flux material property");
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");
  return params;
}

ADOneDEnergyWallHeatFlux::ADOneDEnergyWallHeatFlux(const InputParameters & parameters)
  : ADKernel(parameters),
    _q_wall(getADMaterialProperty<Real>("q_wall")),
    _P_hf(adCoupledValue("P_hf"))
{
}

ADReal
ADOneDEnergyWallHeatFlux::computeQpResidual()
{
  return -_q_wall[_qp] * _P_hf[_qp] * _test[_i][_qp];
}
