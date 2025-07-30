//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentMeshTransformHelper.h"
#include "RotationMatrix.h"
#include "Factory.h"
#include "MooseApp.h"

InputParameters
ComponentMeshTransformHelper::validParams()
{
  auto params = ActionComponent::validParams();

  // If your component is 0D, you should make these two parameters private!
  params.addParam<RealVectorValue>(
      "rotation",
      "Rotation angles (XZX convention, in degrees NOT radians) to rotate the "
      "mesh of this component with. Note that this rotation "
      "is applied before the translation.");
  params.addParam<RealVectorValue>(
      "direction",
      "Direction to orient the component mesh with, assuming it is initially oriented along the "
      "X-axis (1, 0, 0). Note that this rotation is applied before the translation.");
  // Position is widely applicable to components
  params.addParam<Point>(
      "position", Point(0., 0., 0.), "Vector to translate the mesh of this component by.");

  params.addParamNamesToGroup("rotation direction position",
                              "Position and orientation of the component");
  return params;
}

ComponentMeshTransformHelper::ComponentMeshTransformHelper(const InputParameters & params)
  : ActionComponent(params),
    _rotation(queryParam<RealVectorValue>("rotation")),
    _direction(queryParam<RealVectorValue>("direction")),
    _translation(getParam<Point>("position"))
{
  addRequiredTask("add_mesh_generator");
  checkSecondParamNotSetIfFirstOneSet("rotation", "direction");
}

void
ComponentMeshTransformHelper::addMeshGenerators()
{
  // Rotate the mesh as desired
  if (_rotation || _direction)
  {
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _mg_names.back();

    if (_rotation)
    {
      params.set<MooseEnum>("transform") = "ROTATE";
      params.set<RealVectorValue>("vector_value") = *_rotation;
    }
    else
    {
      params.set<MooseEnum>("transform") = "ROTATE_WITH_MATRIX";

      RealVectorValue angles;
      const auto rotation_matrix =
          RotationMatrix::rotationMatrixVecToVec<false>(RealVectorValue(1, 0, 0), *_direction);
      params.set<RealTensorValue>("rotation_matrix") = rotation_matrix;
    }
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "TransformGenerator", name() + "_rotated", params);
    _mg_names.push_back(name() + "_rotated");
  }

  // Add a translation
  {
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _mg_names.back();
    params.set<MooseEnum>("transform") = "TRANSLATE";
    params.set<RealVectorValue>("vector_value") = getParam<Point>("position");
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "TransformGenerator", name() + "_translated", params);
    _mg_names.push_back(name() + "_translated");
  }
}
