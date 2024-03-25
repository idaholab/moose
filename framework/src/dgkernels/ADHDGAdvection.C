//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGAdvection.h"

registerMooseObject("MooseApp", ADHDGAdvection);

InputParameters
ADHDGAdvection::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addRequiredCoupledVar("side_variable", "side variable to use as Lagrange multiplier");
  return params;
}

ADHDGAdvection::ADHDGAdvection(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _velocity_neighbor(getNeighborADMaterialProperty<RealVectorValue>("velocity")),
    _side_u(adCoupledValue("side_variable"))
{
}

ADReal
ADHDGAdvection::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0.0;

  switch (type)
  {
    case Moose::Element:
    {
      const auto vdotn = _velocity[_qp] * _normals[_qp];
      if (MetaPhysicL::raw_value(vdotn) >= 0)
        // outflow
        r = _test[_i][_qp] * vdotn * _u[_qp];
      else
        // inflow
        r = _test[_i][_qp] * vdotn * _side_u[_qp];
      break;
    }

    case Moose::Neighbor:
    {
      const auto vdotn = _velocity_neighbor[_qp] * -_normals[_qp];
      if (MetaPhysicL::raw_value(vdotn) >= 0)
        // outflow
        r = _test_neighbor[_i][_qp] * vdotn * _u_neighbor[_qp];
      else
        // inflow
        r = _test_neighbor[_i][_qp] * vdotn * _side_u[_qp];
      break;
    }
  }

  return r;
}
