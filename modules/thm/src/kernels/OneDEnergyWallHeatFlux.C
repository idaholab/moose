#include "OneDEnergyWallHeatFlux.h"
#include "Function.h"

template<>
InputParameters validParams<OneDEnergyWallHeatFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("q_wall", "Wall heat flux given by a function");
  params.addRequiredCoupledVar("heat_flux_perimeter", "heat flux perimeter");
  return params;
}

OneDEnergyWallHeatFlux::OneDEnergyWallHeatFlux(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _q_wall(getFunction("q_wall")),
    _Phf(coupledValue("heat_flux_perimeter"))
{
}

Real
OneDEnergyWallHeatFlux::computeQpResidual()
{
  return - _q_wall.value(_t, _q_point[_qp]) * _Phf[_qp] * _test[_i][_qp];
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
