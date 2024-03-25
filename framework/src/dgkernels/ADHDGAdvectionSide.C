//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGAdvectionSide.h"

registerMooseObject("MooseApp", ADHDGAdvectionSide);

InputParameters
ADHDGAdvectionSide::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addRequiredCoupledVar("interior_variable", "interior variable to find jumps in");
  return params;
}

ADHDGAdvectionSide::ADHDGAdvectionSide(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _velocity_neighbor(getNeighborADMaterialProperty<RealVectorValue>("velocity")),
    _interior_value(adCoupledValue("interior_variable")),
    _interior_neighbor_value(adCoupledNeighborValue("interior_variable"))
{
}

ADReal
ADHDGAdvectionSide::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0.0;

  switch (type)
  {
    case Moose::Element:
    {
      {
        const auto vdotn = _velocity[_qp] * _normals[_qp];
        if (MetaPhysicL::raw_value(vdotn) >= 0)
          // outflow
          r -= _test[_i][_qp] * vdotn * _interior_value[_qp];
        else
          // inflow
          r -= _test[_i][_qp] * vdotn * _u[_qp];
      }
      {
        const auto vdotn = _velocity_neighbor[_qp] * -_normals[_qp];
        if (MetaPhysicL::raw_value(vdotn) >= 0)
          // outflow
          r -= _test[_i][_qp] * vdotn * _interior_neighbor_value[_qp];
        else
          // inflow
          r -= _test[_i][_qp] * vdotn * _u[_qp];
      }
      break;
    }

    case Moose::Neighbor:
      r = 0;
      break;
  }

  return r;
}
