//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRadiativeHeatFluxRZBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADRadiativeHeatFluxRZBC);

InputParameters
ADRadiativeHeatFluxRZBC::validParams()
{
  InputParameters params = ADRadiativeHeatFluxBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a cylindrical heat structure");

  return params;
}

ADRadiativeHeatFluxRZBC::ADRadiativeHeatFluxRZBC(const InputParameters & parameters)
  : ADRadiativeHeatFluxBC(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADRadiativeHeatFluxRZBC::computeQpResidual()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * ADRadiativeHeatFluxBC::computeQpResidual();
}
