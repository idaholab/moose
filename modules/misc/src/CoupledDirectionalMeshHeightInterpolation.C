/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CoupledDirectionalMeshHeightInterpolation.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template <>
InputParameters
validParams<CoupledDirectionalMeshHeightInterpolation>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("coupled_var",
                               "The variable whose values are going to be interpolated.");

  MooseEnum directions("x y z");
  params.addRequiredParam<MooseEnum>("direction", directions, "The direction to interpolate in.");

  return params;
}

CoupledDirectionalMeshHeightInterpolation::CoupledDirectionalMeshHeightInterpolation(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _coupled_val(coupledValue("coupled_var")),
    _direction(getParam<MooseEnum>("direction"))
{
  MeshTools::BoundingBox bounding_box = MeshTools::bounding_box(_subproblem.mesh());

  _direction_min = bounding_box.min()(_direction);
  _direction_max = bounding_box.max()(_direction);
}

Real
CoupledDirectionalMeshHeightInterpolation::computeValue()
{
  const Node & current_pos = *_current_node;

  Real percentage_along_direction =
      (current_pos(_direction) - _direction_min) / (_direction_max - _direction_min);

  return percentage_along_direction * _coupled_val[_qp];
}
