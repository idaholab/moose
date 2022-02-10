//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureHeatSourceRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureHeatSourceRZ);

InputParameters
HeatStructureHeatSourceRZ::validParams()
{
  InputParameters params = HeatStructureHeatSource::validParams();
  params += RZSymmetry::validParams();
  return params;
}

HeatStructureHeatSourceRZ::HeatStructureHeatSourceRZ(const InputParameters & parameters)
  : HeatStructureHeatSource(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatStructureHeatSourceRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatStructureHeatSource::computeQpResidual();
}

Real
HeatStructureHeatSourceRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatStructureHeatSource::computeQpJacobian();
}
