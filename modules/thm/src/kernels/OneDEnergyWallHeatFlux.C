#include "OneDEnergyWallHeatFlux.h"

registerMooseObject("THMApp", OneDEnergyWallHeatFlux);

template <>
InputParameters
validParams<OneDEnergyWallHeatFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<MaterialPropertyName>("q_wall", "Wall heat flux material property");
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");
  return params;
}

OneDEnergyWallHeatFlux::OneDEnergyWallHeatFlux(const InputParameters & parameters)
  : Kernel(parameters), _q_wall(getMaterialProperty<Real>("q_wall")), _P_hf(coupledValue("P_hf"))
{
}

Real
OneDEnergyWallHeatFlux::computeQpResidual()
{
  return -_q_wall[_qp] * _P_hf[_qp] * _test[_i][_qp];
}

Real
OneDEnergyWallHeatFlux::computeQpJacobian()
{
  return 0.;
}

Real
OneDEnergyWallHeatFlux::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
