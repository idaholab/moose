//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Transform.h"
#include "MooseMesh.h"

#include "libmesh/mesh_modification.h"

template <>
InputParameters
validParams<Transform>()
{
  MooseEnum transforms("TRANSLATE=1 ROTATE=2 SCALE=3");

  InputParameters params = validParams<MeshModifier>();
  params.addClassDescription("Applies a linear transform to the entire mesh.");
  params.addRequiredParam<MooseEnum>(
      "transform", transforms, "The type of transformation to perform (TRANSLATE, ROTATE, SCALE)");
  params.addRequiredParam<RealVectorValue>(
      "vector_value",
      "The value to use for the transformation. When using TRANSLATE or SCALE, the "
      "xyz coordinates are applied in each direction respectively. When using "
      "ROTATE, the values are interpreted as the Euler angles phi, theta and psi "
      "given in degrees.");

  return params;
}

Transform::Transform(const InputParameters & parameters)
  : MeshModifier(parameters),
    _transform(getParam<MooseEnum>("transform")),
    _vector_value(getParam<RealVectorValue>("vector_value"))
{
}

void
Transform::modify()
{
  switch (_transform)
  {
    case 1:
      MeshTools::Modification::translate(
          *_mesh_ptr, _vector_value(0), _vector_value(1), _vector_value(2));
      break;
    case 2:
      MeshTools::Modification::rotate(
          *_mesh_ptr, _vector_value(0), _vector_value(1), _vector_value(2));
      break;
    case 3:
      MeshTools::Modification::scale(
          *_mesh_ptr, _vector_value(0), _vector_value(1), _vector_value(2));
      break;
  }
}
