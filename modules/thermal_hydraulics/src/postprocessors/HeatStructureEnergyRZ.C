//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureEnergyRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureEnergyRZ);

InputParameters
HeatStructureEnergyRZ::validParams()
{
  InputParameters params = HeatStructureEnergyBase::validParams();
  params += RZSymmetry::validParams();
  params.addClassDescription("Computes the total energy for a cylindrical heat structure.");
  return params;
}

HeatStructureEnergyRZ::HeatStructureEnergyRZ(const InputParameters & parameters)
  : HeatStructureEnergyBase(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatStructureEnergyRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatStructureEnergyBase::computeQpIntegral();
}
