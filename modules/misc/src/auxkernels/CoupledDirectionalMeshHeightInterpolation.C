//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledDirectionalMeshHeightInterpolation.h"
#include "MooseMesh.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("MiscApp", CoupledDirectionalMeshHeightInterpolation);

InputParameters
CoupledDirectionalMeshHeightInterpolation::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar(
      "coupled_var",
      "The variable whose values are scaled based on position relative to the model bounds.");

  MooseEnum directions("x y z");
  params.addRequiredParam<MooseEnum>("direction", directions, "The direction to interpolate in.");
  params.addClassDescription(
      "Scales a variable based on position relative to the model bounds in a specified direction");

  return params;
}

CoupledDirectionalMeshHeightInterpolation::CoupledDirectionalMeshHeightInterpolation(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _coupled_val(coupledValue("coupled_var")),
    _direction(getParam<MooseEnum>("direction"))
{
  BoundingBox bounding_box = MeshTools::create_bounding_box(_subproblem.mesh());

  _direction_min = bounding_box.min()(_direction);
  _direction_max = bounding_box.max()(_direction);
}

Real
CoupledDirectionalMeshHeightInterpolation::computeValue()
{
  const Node & current_pos = *_current_node;

  const Real fraction_along_direction =
      (current_pos(_direction) - _direction_min) / (_direction_max - _direction_min);

  return fraction_along_direction * _coupled_val[_qp];
}
