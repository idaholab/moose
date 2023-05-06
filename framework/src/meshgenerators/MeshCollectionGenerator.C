//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCollectionGenerator.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", MeshCollectionGenerator);

InputParameters
MeshCollectionGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Collects multiple meshes into a single (unconnected) mesh.");
  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  return params;
}

MeshCollectionGenerator::MeshCollectionGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _meshes(getMeshes("inputs"))
{
  // error check
  if (_input_names.empty())
    paramError("input_names", "You need to specify at least one MeshGenerator as an input.");
}

std::unique_ptr<MeshBase>
MeshCollectionGenerator::generate()
{
  // merge all meshes into the first one
  auto mesh = dynamic_pointer_cast<UnstructuredMesh>(std::move(*_meshes[0]));
  if (!mesh)
    paramError("inputs", _input_names[0], " is not a valid unstructured mesh");

  // Read in all of the other meshes
  for (MooseIndex(_meshes) i = 1; i < _meshes.size(); ++i)
  {
    auto other_mesh = dynamic_pointer_cast<UnstructuredMesh>(std::move(*_meshes[i]));
    if (!other_mesh)
      paramError("inputs", _input_names[i], " is not a valid unstructured mesh");

    dof_id_type node_delta = mesh->max_node_id();
    dof_id_type elem_delta = mesh->max_elem_id();

    unique_id_type unique_delta =
#ifdef LIBMESH_ENABLE_UNIQUE_ID
        mesh->parallel_max_unique_id();
#else
        0;
#endif

    // Copy mesh data over from the other mesh
    mesh->copy_nodes_and_elements(*other_mesh,
                                  /*skip_find_neighbors = */ false,
                                  elem_delta,
                                  node_delta,
                                  unique_delta);

    // Copy BoundaryInfo from other_mesh too.  We do this via the
    // list APIs rather than element-by-element for speed.
    BoundaryInfo & boundary = mesh->get_boundary_info();
    const BoundaryInfo & other_boundary = other_mesh->get_boundary_info();

    for (const auto & t : other_boundary.build_node_list())
      boundary.add_node(std::get<0>(t) + node_delta, std::get<1>(t));

    for (const auto & t : other_boundary.build_side_list())
      boundary.add_side(std::get<0>(t) + elem_delta, std::get<1>(t), std::get<2>(t));

    for (const auto & t : other_boundary.build_edge_list())
      boundary.add_edge(std::get<0>(t) + elem_delta, std::get<1>(t), std::get<2>(t));

    for (const auto & t : other_boundary.build_shellface_list())
      boundary.add_shellface(std::get<0>(t) + elem_delta, std::get<1>(t), std::get<2>(t));

    const auto & boundary_ids = boundary.get_boundary_ids();
    const auto & other_boundary_ids = other_boundary.get_boundary_ids();
    for (auto id : other_boundary_ids)
    {
      // check if the boundary id already exists with a different name
      if (boundary_ids.count(id))
      {
        if (boundary.get_sideset_name(id) != "" &&
            boundary.get_sideset_name(id) != other_boundary.get_sideset_name(id))
          mooseError("A sideset with id ",
                     id,
                     " exists but has different names in the merged meshes ('",
                     boundary.get_sideset_name(id),
                     "' vs.'",
                     other_boundary.get_sideset_name(id),
                     "').");

        boundary.sideset_name(id) = other_boundary.get_sideset_name(id);

        if (boundary.get_nodeset_name(id) != "" &&
            boundary.get_nodeset_name(id) != other_boundary.get_nodeset_name(id))
          mooseError("A nodeset with id ",
                     id,
                     " exists but has different names in the merged meshes ('",
                     boundary.get_nodeset_name(id),
                     "' vs.'",
                     other_boundary.get_nodeset_name(id),
                     "').");

        boundary.nodeset_name(id) = other_boundary.get_nodeset_name(id);
      }
    }
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
