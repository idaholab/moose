//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchedMeshGenerator.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"

#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", StitchedMeshGenerator);

InputParameters
StitchedMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");

  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  params.addParam<bool>(
      "clear_stitched_boundary_ids", true, "Whether or not to clear the stitched boundary IDs");
  params.addRequiredParam<std::vector<std::vector<std::string>>>(
      "stitch_boundaries_pairs",
      "Pairs of boundaries to be stitched together between the 1st mesh in inputs and each "
      "consecutive mesh");
  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addParam<bool>("prevent_boundary_ids_overlap",
                        true,
                        "Whether to re-number boundaries in stitched meshes to prevent merging of "
                        "unrelated boundaries");
  params.addClassDescription(
      "Allows multiple mesh files to be stitched together to form a single mesh.");

  return params;
}

StitchedMeshGenerator::StitchedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _mesh_ptrs(getMeshes("inputs")),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _clear_stitched_boundary_ids(getParam<bool>("clear_stitched_boundary_ids")),
    _stitch_boundaries_pairs(
        getParam<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs")),
    _algorithm(parameters.get<MooseEnum>("algorithm")),
    _prevent_boundary_ids_overlap(getParam<bool>("prevent_boundary_ids_overlap"))
{
}

std::unique_ptr<MeshBase>
StitchedMeshGenerator::generate()
{
  // We put the first mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh = dynamic_pointer_cast<UnstructuredMesh>(*_mesh_ptrs[0]);
  if (!mesh) // This should never happen until libMesh implements on-the-fly-Elem mesh types
    mooseError("StitchedMeshGenerator is only implemented for unstructured meshes");

  // Reserve spaces for the other meshes (no need to store the first one another time)
  std::vector<std::unique_ptr<UnstructuredMesh>> meshes(_mesh_ptrs.size() - 1);

  // Read in all of the other meshes
  for (MooseIndex(_mesh_ptrs) i = 1; i < _mesh_ptrs.size(); ++i)
    meshes[i - 1] = dynamic_pointer_cast<UnstructuredMesh>(*_mesh_ptrs[i]);

  // Stitch all the meshes to the first one
  for (MooseIndex(meshes) i = 0; i < meshes.size(); i++)
  {
    auto boundary_pair = _stitch_boundaries_pairs[i];

    boundary_id_type first, second;

    try
    {
      first = MooseUtils::convert<boundary_id_type>(boundary_pair[0], true);
    }
    catch (...)
    {
      first = mesh->get_boundary_info().get_id_by_name(boundary_pair[0]);

      if (first == BoundaryInfo::invalid_id)
      {
        std::stringstream error;

        error << "Boundary " << boundary_pair[0] << " doesn't exist on the first mesh in " << name()
              << "\n";
        error << "Boundary names that do exist: \n";
        error << " ID : Name\n";

        auto & sideset_id_name_map = mesh->get_boundary_info().get_sideset_name_map();

        for (auto & ss_name_map_pair : sideset_id_name_map)
          error << " " << ss_name_map_pair.first << " : " << ss_name_map_pair.second << "\n";

        paramError("stitch_boundaries_pairs", error.str());
      }
    }

    try
    {
      second = MooseUtils::convert<boundary_id_type>(boundary_pair[1], true);
    }
    catch (...)
    {
      second = meshes[i]->get_boundary_info().get_id_by_name(boundary_pair[1]);

      if (second == BoundaryInfo::invalid_id)
      {
        meshes[i]->print_info();

        std::stringstream error;

        error << "Boundary " << boundary_pair[1] << " doesn't exist on mesh " << i + 1 << " in "
              << name() << "\n";
        error << "Boundary names that do exist: \n";
        error << " ID : Name\n";

        auto & sideset_id_name_map = meshes[i]->get_boundary_info().get_sideset_name_map();

        for (auto & ss_name_map_pair : sideset_id_name_map)
          error << " " << ss_name_map_pair.first << " : " << ss_name_map_pair.second << "\n";

        paramError("stitch_boundaries_pairs", error.str());
      }
    }

    const bool use_binary_search = (_algorithm == "BINARY");

    // Avoid chaotic boundary merging due to overlapping ids in meshes by simply renumbering the
    // boundaries in the meshes we are going to stitch onto the base mesh
    // This is only done if there is an overlap
    if (_prevent_boundary_ids_overlap)
    {
      // Meshes must be prepared to get the global boundary ids and global boundary ids
      // must be used in parallel to be sure to renumber boundaries consistently across processes
      if (!mesh->is_prepared())
        mesh->prepare_for_use();
      if (!meshes[i]->is_prepared())
        meshes[i]->prepare_for_use();

      const auto & base_mesh_bids = mesh->get_boundary_info().get_global_boundary_ids();
      const auto stitched_mesh_bids = meshes[i]->get_boundary_info().get_global_boundary_ids();
      // Check for an overlap
      bool overlap_found = false;
      for (const auto & bid : stitched_mesh_bids)
        if (base_mesh_bids.count(bid))
          overlap_found = true;

      if (overlap_found)
      {
        const auto max_boundary_id = *base_mesh_bids.rbegin();
        BoundaryID new_index = 0;
        for (const auto bid : stitched_mesh_bids)
        {
          const auto new_bid = max_boundary_id + (new_index++);
          meshes[i]->get_boundary_info().renumber_id(bid, new_bid);
          if (bid == second)
            second = new_bid;
        }
      }
    }

    mesh->stitch_meshes(*meshes[i],
                        first,
                        second,
                        TOLERANCE,
                        _clear_stitched_boundary_ids,
                        /*verbose = */ true,
                        use_binary_search);
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
