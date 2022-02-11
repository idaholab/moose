//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectionHeatTransferRZBC.h"

registerMooseObject("ThermalHydraulicsApp", ADConvectionHeatTransferRZBC);

InputParameters
ADConvectionHeatTransferRZBC::validParams()
{
  InputParameters params = ADConvectionHeatTransferBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription("Convection BC for RZ domain in XY coordinate system");

  return params;
}

ADConvectionHeatTransferRZBC::ADConvectionHeatTransferRZBC(const InputParameters & parameters)
  : ADConvectionHeatTransferBC(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADConvectionHeatTransferRZBC::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADConvectionHeatTransferBC::computeQpResidual();
}
