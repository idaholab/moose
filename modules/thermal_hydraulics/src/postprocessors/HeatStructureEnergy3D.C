//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureEnergy3D.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureEnergy3D);

InputParameters
HeatStructureEnergy3D::validParams()
{
  InputParameters params = HeatStructureEnergyBase::validParams();
  params.suppressParameter<Real>("n_units");
  params.addClassDescription("Computes the total energy for a 3D heat structure.");
  return params;
}

HeatStructureEnergy3D::HeatStructureEnergy3D(const InputParameters & parameters)
  : HeatStructureEnergyBase(parameters)
{
}

Real
HeatStructureEnergy3D::computeQpIntegral()
{
  return HeatStructureEnergyBase::computeQpIntegral();
}
