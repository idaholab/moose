//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DCouplerRZBC.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructure2DCouplerRZBC);

InputParameters
HeatStructure2DCouplerRZBC::validParams()
{
  InputParameters params = HeatStructure2DCouplerBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Applies BC for HeatStructure2DCoupler for cylindrical heat structure");

  return params;
}

HeatStructure2DCouplerRZBC::HeatStructure2DCouplerRZBC(const InputParameters & parameters)
  : HeatStructure2DCouplerBC(parameters), RZSymmetry(this, parameters)
{
}

ADReal
HeatStructure2DCouplerRZBC::computeQpResidual()
{
  const Real P_heat = computeCircumference(_q_point[_qp]);
  return P_heat * HeatStructure2DCouplerBC::computeQpResidual();
}
