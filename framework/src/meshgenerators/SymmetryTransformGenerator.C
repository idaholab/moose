//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetryTransformGenerator.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "libmesh/mesh_modification.h"

registerMooseObject("MooseApp", SymmetryTransformGenerator);

InputParameters
SymmetryTransformGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Applies a symmetry transformation to the entire mesh.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<RealEigenVector>(
      "mirror_point",
      "Any point on the plane/line over which the reflection operation will be done");
  params.addRequiredParam<RealEigenVector>(
      "mirror_normal_vector",
      "A vector normal to (perpendicular/orthogonal to) the plane/line over which the "
      "reflection operation will be done");

  return params;
}

SymmetryTransformGenerator::SymmetryTransformGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _mirror_point_vector(getParam<RealEigenVector>("mirror_point")),
    _mirror_normal_vector(getParam<RealEigenVector>("mirror_normal_vector"))
{
  // enforce 3D coordinates
  if (_mirror_point_vector.size() != 3)
    paramError("mirror_point",
               "mirror_point should be a 3d vector, but only ",
               _mirror_point_vector.size(),
               "components were specified.");

  if (_mirror_normal_vector.size() != 3)
    paramError("mirror_point",
               " mirror_normal_vector should be a 3d vector, but only ",
               _mirror_normal_vector.size(),
               "components were specified.");

  // convert normal vector into a unit normal vector
  const auto norm = _mirror_normal_vector.norm();
  if (!MooseUtils::absoluteFuzzyEqual(norm, 1))
    mooseInfo("Input normal plane vector was not normalized, normalization was performed");
  _mirror_normal_vector = _mirror_normal_vector / norm;
}

std::unique_ptr<MeshBase>
SymmetryTransformGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // https://en.wikipedia.org/wiki/Transformation_matrix#Reflection_2
  // variables renamed for readability and verification
  const auto a = _mirror_normal_vector[0], b = _mirror_normal_vector[1],
             c = _mirror_normal_vector[2],
             d = ((RealEigenVector)(-1 * _mirror_point_vector.transpose() * _mirror_normal_vector))(
                 0);

  RealEigenMatrix mirror_transformation(4, 4);
  mirror_transformation << (1 - 2 * a * a), (-2 * a * b), (-2 * a * c), (-2 * a * d), (-2 * a * b),
      (1 - 2 * b * b), (-2 * b * c), (-2 * b * d), (-2 * a * c), (-2 * b * c), (1 - 2 * c * c),
      (-2 * c * d), (0), (0), (0), (1);

  for (auto & node : mesh->node_ptr_range())
  {
    RealEigenVector location_vec(4);
    location_vec << (*node)(0), (*node)(1), (*node)(2), 1;
    location_vec = mirror_transformation * location_vec;

    (*node)(0) = location_vec(0);
    (*node)(1) = location_vec(1);
    (*node)(2) = location_vec(2);
  }

  // Fix flipped orientation from the symmetry
  MeshTools::Modification::orient_elements(*mesh);

  return mesh;
}
