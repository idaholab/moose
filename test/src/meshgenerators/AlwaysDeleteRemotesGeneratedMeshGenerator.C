//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AlwaysDeleteRemotesGeneratedMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/elem.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("MooseTestApp", AlwaysDeleteRemotesGeneratedMeshGenerator);

InputParameters
AlwaysDeleteRemotesGeneratedMeshGenerator::validParams()
{
  return GeneratedMeshGenerator::validParams();
}

AlwaysDeleteRemotesGeneratedMeshGenerator::AlwaysDeleteRemotesGeneratedMeshGenerator(
    const InputParameters & parameters)
  : GeneratedMeshGenerator(parameters)
{
}

std::unique_ptr<MeshBase>
AlwaysDeleteRemotesGeneratedMeshGenerator::generate()
{
  // Have MOOSE construct the correct libMesh::Mesh object using Mesh block and CLI parameters.
  auto mesh = GeneratedMeshGenerator::generate();

  // always delete remote elements for this test object
  mesh->delete_remote_elements();

  // carry out the mesh preparation steps after remote element deletion
  mesh->get_boundary_info().regenerate_id_sets();

  if (mesh->allow_renumbering())
    mesh->renumber_nodes_and_elements();

  return mesh;
}
