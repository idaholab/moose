//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  MooseEnum transforms("TRANSLATE=1 TRANSLATE_CENTER_ORIGIN=2 TRANSLATE_MIN_ORIGIN=3 ROTATE=4 "
                       "SCALE=5 ROTATE_WITH_MATRIX=6 ROTATE_EXT=7");

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
      "given in degrees. For ROTATE_EXT, and extrinsic rotation is carried out using prescribed "
      "angles alpha, beta, and gamma in degrees.");
  params.addParam<RealTensorValue>("rotation_matrix",
                                   "Precomputed extrinsic rotation matrix to be applied to mesh.");

  return params;
}

TransformGenerator::TransformGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _transform(getParam<MooseEnum>("transform"))
{
  if ((_transform != "TRANSLATE_CENTER_ORIGIN" && _transform != "TRANSLATE_MIN_ORIGIN" &&
       _transform != "ROTATE_WITH_MATRIX") &&
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
  RealTensorValue rotation_matrix;
  if (_transform == 2 || _transform == 3)
  {
    const auto bbox = MeshTools::create_bounding_box(*mesh);
    if (_transform == 2)
      vector_value = -0.5 * (bbox.max() + bbox.min());
    else
      vector_value = -bbox.min();
  }
  else if (_transform == 6)
    rotation_matrix = getParam<RealTensorValue>("rotation_matrix");
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
    case 6:
      TransformGenerator::rotateWithMatrix(*mesh, rotation_matrix);
      break;
    case 7:
      TransformGenerator::rotateExtrinsic(*mesh, vector_value(0), vector_value(1), vector_value(2));
      break;
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
TransformGenerator::rotateExtrinsic(MeshBase & mesh,
                                    const Real alpha,
                                    const Real beta,
                                    const Real gamma)
{
#if LIBMESH_DIM == 3
  const auto R = RealTensorValue::extrinsic_rotation_matrix(
      alpha, beta, gamma); // this line is the only difference from rotate...

  if (beta)
    mesh.set_spatial_dimension(3);

  for (auto & node : mesh.node_ptr_range())
  {
    Point & pt = *node;
    pt = R * pt;
  }
#else
  libmesh_ignore(mesh, alpha, beta, gamma);
  libmesh_error_msg(
      "MeshTools::Modification::rotate() requires libMesh to be compiled with LIBMESH_DIM==3");
#endif
}

void
TransformGenerator::rotateWithMatrix(MeshBase & mesh,
                                     const GenericRealTensorValue<false> & rotation_matrix)
{
#if LIBMESH_DIM == 3
  mesh.set_spatial_dimension(3);

  for (auto & node : mesh.node_ptr_range())
  {
    Point & pt = *node;
    pt = rotation_matrix * pt;
  }

#else
  libmesh_ignore(mesh, phi, theta, psi);
  libmesh_error_msg(
      "MeshTools::Modification::rotate() requires libMesh to be compiled with LIBMESH_DIM==3");
#endif
}
