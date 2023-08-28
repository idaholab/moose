//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDGAdvection.h"

registerMooseObject("MooseApp", ADDGAdvection);

InputParameters
ADDGAdvection::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addClassDescription(
      "Adds internal face advection flux contributions for discontinuous Galerkin discretizations");
  params.addParam<MaterialPropertyName>("advected_quantity",
                                        "An optional material property to be advected. If not "
                                        "supplied, then the variable will be used.");
  return params;
}

ADDGAdvection::ADDGAdvection(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _velocity_neighbor(getNeighborADMaterialProperty<RealVectorValue>("velocity")),
    _adv_quant_elem(isParamValid("advected_quantity")
                        ? getADMaterialProperty<Real>("advected_quantity").get()
                        : _u),
    _adv_quant_neighbor(isParamValid("advected_quantity")
                            ? getNeighborADMaterialProperty<Real>("advected_quantity").get()
                            : _u_neighbor)
{
}

ADReal
ADDGAdvection::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0;

  auto average = [](const auto & elem_value, const auto & neighbor_value)
  { return (elem_value + neighbor_value) / 2; };

  const auto vdotn = average(_velocity[_qp], _velocity_neighbor[_qp]) * _normals[_qp];

  const auto face_u = [&]()
  {
    if (vdotn >= 0)
      return _adv_quant_elem[_qp];
    else
      return _adv_quant_neighbor[_qp];
  }();

  switch (type)
  {
    case Moose::Element:
      r += vdotn * face_u * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      r -= vdotn * face_u * _test_neighbor[_i][_qp];
      break;
  }

  return r;
}
