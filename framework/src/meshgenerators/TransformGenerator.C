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

#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", TransformGenerator);

InputParameters
TransformGenerator::validParams()
{
  MooseEnum transforms(
      "TRANSLATE=1 TRANSLATE_CENTER_ORIGIN=2 TRANSLATE_MIN_ORIGIN=3 ROTATE=4 SCALE=5");

  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Applies a linear transform to the entire mesh.");
  params.addRequiredParam<MooseEnum>(
      "transform",
      transforms,
      "The type of transformation to perform (TRANSLATE, TRANSLATE_CENTER_ORIGIN, "
      "TRANSLATE_MIN_ORIGIN, ROTATE, SCALE)");
  params.addParam<RealVectorValue>(
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
    _transform(getParam<MooseEnum>("transform"))
{
  if ((_transform != "TRANSLATE_CENTER_ORIGIN" && _transform != "TRANSLATE_MIN_ORIGIN") &&
      !isParamValid("vector_value"))
    paramError("transform",
               "The parameter 'vector_value' must be supplied with 'transform' = ",
               _transform);
}

std::unique_ptr<MeshBase>
TransformGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  RealVectorValue vector_value;
  if (_transform == 2 || _transform == 3)
  {
    const auto bbox = MeshTools::create_bounding_box(*mesh);
    if (_transform == 2)
      vector_value = -0.5 * (bbox.max() + bbox.min());
    else
      vector_value = -bbox.min();
  }
  else
    vector_value = getParam<RealVectorValue>("vector_value");

  switch (_transform)
  {
    case 1:
    case 2:
    case 3:
      MeshTools::Modification::translate(*mesh, vector_value(0), vector_value(1), vector_value(2));
      break;
    case 4:
      MeshTools::Modification::rotate(*mesh, vector_value(0), vector_value(1), vector_value(2));
      break;
    case 5:
      MeshTools::Modification::scale(*mesh, vector_value(0), vector_value(1), vector_value(2));
      break;
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
