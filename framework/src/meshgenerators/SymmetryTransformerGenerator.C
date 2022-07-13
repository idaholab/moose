//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetryTransformerGenerator.h"
#include "CastUniquePointer.h"


registerMooseObject("MooseApp", SymmetryTransformerGenerator);

InputParameters
SymmetryTransformerGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Applies a mirror transform to the entire mesh.");
  params.addRequiredParam<RealEigenVector>(
      "mirror_point",
      "Any point on the plane/line over which the reflection operation will be done");
  params.addRequiredParam<RealEigenVector>(
      "mirror_normal_vector",
      "A vector normal to (perpendicular/orthogonal to) the plane/line over which the "
      "reflection operation will be done");
  params.addParam<std::vector<std::vector<std::string>>>(
      " ",
      std::vector<std::vector<std::string>>(),
      "Pairs of boundaries to be stitched together between the 1st mesh in inputs and each "
      "consecutive mesh");
  return params;
}

SymmetryTransformerGenerator::SymmetryTransformerGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _mirror_point_vector(getParam<RealEigenVector>("mirror_point")),
    _mirror_normal_vector(getParam<RealEigenVector>("mirror_normal_vector")),
    _stitch_boundaries_pairs(
        getParam<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs"))
{
  // enforce 3D coordinates
  if (_mirror_point_vector.size() != 3)
    mooseError("mirror_point should be 3d vectors, but a dimension of ",
               _mirror_point_vector.size(),
               "was entered instead, respectively. If you are working in 2D, let the 3rd component"
               "of your vectors be zero.");

  if (_mirror_normal_vector.size() != 3)
    mooseError(" mirror_normal_vector should be 3d vectors, but a dimension of ",
               _mirror_normal_vector.size(),
               "was entered instead, respectively. If you are working in 2D, let the 3rd component"
               "of your vectors be zero.");

  // convert normal vector into a unit normal vector
  double nrm = _mirror_normal_vector.norm();
  _mirror_normal_vector << _mirror_normal_vector[0] / nrm, _mirror_normal_vector[1] / nrm,
      _mirror_normal_vector[2] / nrm;
}

std::unique_ptr<MeshBase>
SymmetryTransformerGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // https://en.wikipedia.org/wiki/Transformation_matrix#Reflection_2
  // variables renamed for readability and verification
  double a = _mirror_normal_vector[0], b = _mirror_normal_vector[1], c = _mirror_normal_vector[2],
         d = ((RealEigenVector)(-1 * _mirror_point_vector.transpose() * _mirror_normal_vector))(0);

  RealEigenMatrix mirror_transformation(4, 4);
  mirror_transformation << (1 - 2 * a * a), (-2 * a * b), (-2 * a * c), (-2 * a * d), (-2 * a * b),
      (1 - 2 * b * b), (-2 * b * c), (-2 * b * d), (-2 * a * c), (-2 * b * c), (1 - 2 * c * c),
      (-2 * c * d), (0), (0), (0), (1);

  for (auto & node : mesh->node_ptr_range())
  {
    RealEigenVector location_vec(4);
    location_vec << (*node)(0), (*node)(1), (*node)(2), 1;

    location_vec = mirror_transformation * location_vec;

    // MOOSE uses libmesh in a 3 dim only mode, therefore we assume 3D here
    (*node)(0) = location_vec(0);
    (*node)(1) = location_vec(1);
    (*node)(2) = location_vec(2);
  }

  if (_stitch_boundaries_pairs.size() > 0)
  {
    auto params = _app.getFactory().getValidParams("StitchedMeshGenerator");

    // order of vector elements matters for this generator
    // here order by: original mesh first, our custom mesh second
    params.set<std::vector<MeshGeneratorName>>("inputs") = {getParam<MeshGeneratorName>("input"),
                                                            name()};

    // params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
    //     {_sideset_name, _SIDESET_TO_BE_STITCHED}};
    params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") =
        _stitch_boundaries_pairs;

    // stitch the newly made high-dimensional mesh back to the original mesh
    return dynamic_pointer_cast<MeshBase>(
        addMeshSubgenerator("StitchedMeshGenerator", name() + "_stitchedMeshGenerator", params));
  }

  return mesh;
}
