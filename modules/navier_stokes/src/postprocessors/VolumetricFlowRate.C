//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumetricFlowRate.h"
#include <math.h>

template <>
InputParameters
validParams<VolumetricFlowRate>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addClassDescription("Computes the volumetric flow rate through a boundary.");
  params.addRequiredCoupledVar("vel_x", "The x velocity");
  params.addCoupledVar("vel_y", 0, "The y velocity");
  params.addCoupledVar("vel_z", 0, "The z velocity");
  return params;
}

VolumetricFlowRate::VolumetricFlowRate(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _vel_x(coupledValue("vel_x")),
    _vel_y(coupledValue("vel_y")),
    _vel_z(coupledValue("vel_z"))
{
}

Real
VolumetricFlowRate::computeQpIntegral()
{
  return RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * _normals[_qp];
}
