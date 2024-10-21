//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatFlux3EqnBC.h"
#include "ADHeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndicesVACE.h"
#include "Assembly.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatFlux3EqnBC);

InputParameters
ADHeatFlux3EqnBC::validParams()
{
  InputParameters params = ADHeatFluxBaseBC::validParams();
  params.addClassDescription("Wall heat flux boundary condition for the energy equation");
  return params;
}

ADHeatFlux3EqnBC::ADHeatFlux3EqnBC(const InputParameters & parameters)
  : ADHeatFluxBaseBC(parameters)
{
}

ADReal
ADHeatFlux3EqnBC::computeQpResidual()
{
  const std::vector<ADReal> & q_wall = _q_uo.getHeatFlux(_current_elem->id());
  const std::vector<ADReal> & P_hf = _q_uo.getHeatedPerimeter(_current_elem->id());
  return -_hs_scale * q_wall[_qp] * P_hf[_qp] * _test[_i][_qp];
}
