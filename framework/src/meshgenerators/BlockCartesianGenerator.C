//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockCartesianGenerator.h"
#include "CastUniquePointer.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"

registerMooseObject("MooseApp", BlockCartesianGenerator);

InputParameters
BlockCartesianGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  params.addRequiredParam<std::vector<Real>>("dx", "Intervals in the X direction");
  params.addParam<std::vector<unsigned int>>(
      "ix", "Number of grids in all intervals in the X direction (default to all one)");
  params.addParam<std::vector<Real>>(
      "dy", "Intervals in the Y direction (required when dim>1 otherwise ignored)");
  params.addParam<std::vector<unsigned int>>(
      "iy", "Number of grids in all intervals in the Y direction (default to all one)");
  params.addParam<std::vector<Real>>(
      "dz", "Intervals in the Z direction (required when dim>2 otherwise ignored)");
  params.addParam<std::vector<unsigned int>>(
      "iz", "Number of grids in all intervals in the Z direction (default to all one)");
  params.addParam<std::vector<unsigned int>>("subdomain_id", "Block IDs (default to all zero)");

  params.addClassDescription("This BlockCartesianGenerator creates a non-uniform Cartesian mesh in "
                             "the mesh block region.");
  return params;
}

BlockCartesianGenerator::BlockCartesianGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _dx(getParam<std::vector<Real>>("dx")),
    _mesh_input(getMesh("input"))
{

  auto params = _app.getFactory().getValidParams("CartesianMeshGenerator");

  params.set<MooseEnum>("dim") = _dim;

  if (isParamValid("ix"))
    params.set<std::vector<unsigned int>>("ix") = getParam<std::vector<unsigned int>>("ix");
  if (isParamValid("iy"))
    params.set<std::vector<unsigned int>>("iy") = getParam<std::vector<unsigned int>>("iy");
  if (isParamValid("iz"))
    params.set<std::vector<unsigned int>>("iz") = getParam<std::vector<unsigned int>>("iz");

  if (isParamValid("dx"))
    params.set<std::vector<Real>>("dx") = getParam<std::vector<Real>>("dx");
  if (isParamValid("dy"))
    params.set<std::vector<Real>>("dy") = getParam<std::vector<Real>>("dy");
  if (isParamValid("dz"))
    params.set<std::vector<Real>>("dz") = getParam<std::vector<Real>>("dz");
  if (isParamValid("subdomain_id"))
    params.set<std::vector<unsigned int>>("subdomain_id") =
        getParam<std::vector<unsigned int>>("subdomain_id");

  // generate lower dimensional mesh from the given sideset
  _build_mesh =
      &addMeshSubgenerator("CartesianMeshGenerator", name() + "_cartesianmeshgenerator", params);
}

std::unique_ptr<MeshBase>
BlockCartesianGenerator::generate()
{
  auto bbox_input = MeshTools::create_bounding_box(*_mesh_input);
  //  auto bbox = MeshTools::create_bounding_box(_build_mesh);

  // Real diff = 0;
  // for (auto i = 0; i < _dim; i++)
  // {
  //   diff = std::abs(bbox_input.max()(i) - bbox_input.min()(i) - bbox.max()(i) + bbox.min()(i));
  //   if (diff > TOLERANCE)
  //     mooseError("The Intervals in ", i, "th dimension doesn't match!");
  // }

  return std::move(*_build_mesh);
}
