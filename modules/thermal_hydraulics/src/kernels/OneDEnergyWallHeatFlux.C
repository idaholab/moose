//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDEnergyWallHeatFlux.h"

registerMooseObject("ThermalHydraulicsApp", OneDEnergyWallHeatFlux);

InputParameters
OneDEnergyWallHeatFlux::validParams()
{
  InputParameters params = Kernel::validParams();
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
