//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatStructureHeatSourceRZ.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatStructureHeatSourceRZ);

InputParameters
ADHeatStructureHeatSourceRZ::validParams()
{
  InputParameters params = ADHeatStructureHeatSource::validParams();
  params += RZSymmetry::validParams();
  return params;
}

ADHeatStructureHeatSourceRZ::ADHeatStructureHeatSourceRZ(const InputParameters & parameters)
  : ADHeatStructureHeatSource(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADHeatStructureHeatSourceRZ::computeQpResidual()
{
  const ADReal circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatStructureHeatSource::computeQpResidual();
}
