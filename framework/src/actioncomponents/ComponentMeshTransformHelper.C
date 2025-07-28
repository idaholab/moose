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
    params.set<MooseEnum>("transform") = "ROTATE";
    if (_rotation)
      params.set<RealVectorValue>("vector_value") = *_rotation;
    else
    {
      const auto rotation_matrix =
          RotationMatrix::rotVec1ToVec2<false>(RealVectorValue(1, 0, 0), -*_direction);
      RealVectorValue angles;
      // angles(0) = std::atan2(rotation_matrix(1, 0), rotation_matrix(0, 0));
      // angles(1) = std::asin(-rotation_matrix(2, 0));
      // angles(2) = std::atan2(rotation_matrix(2, 1), rotation_matrix(2, 2));
      angles(1) = std::asin(-rotation_matrix(0, 2));
      angles(0) = std::asin(rotation_matrix(0, 1) / std::cos(angles(1)));
      angles(2) = std::acos(rotation_matrix(2, 2) / std::cos(angles(1)));
      params.set<RealVectorValue>("vector_value") = angles / M_PI_2 * 90;
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
