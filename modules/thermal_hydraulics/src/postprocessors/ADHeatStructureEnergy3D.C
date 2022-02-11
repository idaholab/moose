//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatStructureEnergy3D.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatStructureEnergy3D);

InputParameters
ADHeatStructureEnergy3D::validParams()
{
  InputParameters params = ADHeatStructureEnergyBase::validParams();
  params.suppressParameter<Real>("n_units");
  params.addClassDescription("Computes the total energy for a 3D heat structure.");
  return params;
}

ADHeatStructureEnergy3D::ADHeatStructureEnergy3D(const InputParameters & parameters)
  : ADHeatStructureEnergyBase(parameters)
{
}

Real
ADHeatStructureEnergy3D::computeQpIntegral()
{
  return ADHeatStructureEnergyBase::computeQpIntegral();
}
