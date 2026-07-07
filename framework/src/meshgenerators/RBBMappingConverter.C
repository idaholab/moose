//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RBBMappingConverter.h"

#include "CastUniquePointer.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", RBBMappingConverter);

InputParameters
RBBMappingConverter::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "input", "Input mesh to convert to use Rational Bernstein-Bezier mappings");

  params.addClassDescription(
      "Reinterpolates all elements in a mesh to use Rational Bernstein-Bezier mappings");

  return params;
}

RBBMappingConverter::RBBMappingConverter(const InputParameters & parameters)
  : MeshGenerator(parameters), _input_ptr(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
RBBMappingConverter::generate()
{
  // Put the input mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_input_ptr));

  MeshTools::Modification::all_rbb(*mesh);

  // The remapping here subtly changes element geometry.  libMesh
  // should be properly clearing any point locator accordingly, but
  // until that's fixed we should clear it ourselves, and even
  // afterward this is a cheap redundancy.
  mesh->clear_point_locator();

  return mesh;
}
