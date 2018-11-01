//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransformGenerator.h"
#include "libmesh/mesh_modification.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", TransformGenerator);

template <>
InputParameters
validParams<TransformGenerator>()
{
  MooseEnum transforms("TRANSLATE=1 ROTATE=2 SCALE=3");

  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
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

TransformGenerator::TransformGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _transform(getParam<MooseEnum>("transform")),
    _vector_value(getParam<RealVectorValue>("vector_value"))
{
}

std::unique_ptr<MeshBase>
TransformGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  switch (_transform)
  {
    case 1:
      MeshTools::Modification::translate(
          *mesh, _vector_value(0), _vector_value(1), _vector_value(2));
      break;
    case 2:
      MeshTools::Modification::rotate(*mesh, _vector_value(0), _vector_value(1), _vector_value(2));
      break;
    case 3:
      MeshTools::Modification::scale(*mesh, _vector_value(0), _vector_value(1), _vector_value(2));
      break;
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
