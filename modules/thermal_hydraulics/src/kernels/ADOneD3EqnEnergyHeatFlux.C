//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneD3EqnEnergyHeatFlux.h"
#include "ADHeatFluxFromHeatStructureBaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ADOneD3EqnEnergyHeatFlux);

InputParameters
ADOneD3EqnEnergyHeatFlux::validParams()
{
  InputParameters params = ADOneDHeatFluxBase::validParams();
  return params;
}

ADOneD3EqnEnergyHeatFlux::ADOneD3EqnEnergyHeatFlux(const InputParameters & parameters)
  : ADOneDHeatFluxBase(parameters)
{
}

ADReal
ADOneD3EqnEnergyHeatFlux::computeQpResidual()
{
  const std::vector<ADReal> & q_wall = _q_uo.getHeatFlux(_current_elem->id());
  const std::vector<ADReal> & P_hf = _q_uo.getHeatedPerimeter(_current_elem->id());
  return -q_wall[_qp] * P_hf[_qp] * _test[_i][_qp];
}
