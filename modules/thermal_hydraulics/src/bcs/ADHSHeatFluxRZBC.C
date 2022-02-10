//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHSHeatFluxRZBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADHSHeatFluxRZBC);

InputParameters
ADHSHeatFluxRZBC::validParams()
{
  InputParameters params = ADHSHeatFluxBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Applies a specified heat flux to the side of a cylindrical heat structure");

  return params;
}

ADHSHeatFluxRZBC::ADHSHeatFluxRZBC(const InputParameters & parameters)
  : ADHSHeatFluxBC(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADHSHeatFluxRZBC::computeQpResidual()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * ADHSHeatFluxBC::computeQpResidual();
}
