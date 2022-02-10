//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalAppConvectionHeatTransferRZBC.h"

registerMooseObject("ThermalHydraulicsApp", ExternalAppConvectionHeatTransferRZBC);

InputParameters
ExternalAppConvectionHeatTransferRZBC::validParams()
{
  InputParameters params = ExternalAppConvectionHeatTransferBC::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Convection BC from an external application for RZ domain in XY coordinate system");

  return params;
}

ExternalAppConvectionHeatTransferRZBC::ExternalAppConvectionHeatTransferRZBC(
    const InputParameters & parameters)
  : ExternalAppConvectionHeatTransferBC(parameters), RZSymmetry(this, parameters)
{
}

Real
ExternalAppConvectionHeatTransferRZBC::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ExternalAppConvectionHeatTransferBC::computeQpResidual();
}

Real
ExternalAppConvectionHeatTransferRZBC::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ExternalAppConvectionHeatTransferBC::computeQpJacobian();
}
