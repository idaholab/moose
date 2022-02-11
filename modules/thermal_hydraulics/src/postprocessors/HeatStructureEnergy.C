//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureEnergy.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureEnergy);

InputParameters
HeatStructureEnergy::validParams()
{
  InputParameters params = HeatStructureEnergyBase::validParams();

  params.addClassDescription("Computes the total energy for a plate heat structure.");

  params.addRequiredParam<Real>("plate_depth", "Depth of the heat structure if plate-type");

  return params;
}

HeatStructureEnergy::HeatStructureEnergy(const InputParameters & parameters)
  : HeatStructureEnergyBase(parameters), _plate_depth(getParam<Real>("plate_depth"))
{
  addMooseVariableDependency(_T_var);
}

Real
HeatStructureEnergy::computeQpIntegral()
{
  return HeatStructureEnergyBase::computeQpIntegral() * _plate_depth;
}
