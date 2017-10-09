/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
