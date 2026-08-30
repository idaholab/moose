//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchMeshGenerator.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/unstructured_mesh.h"

registerMooseObjectRenamed("MooseApp",
                           StitchedMeshGenerator,
                           "06/30/2026 24:00",
                           StitchMeshGenerator);
registerMooseObject("MooseApp", StitchMeshGenerator);

InputParameters
StitchMeshGenerator::validParams()
{
  InputParameters params = StitchMeshGeneratorBase::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  params.addParam<bool>("prevent_boundary_ids_overlap",
                        true,
                        "Whether to re-number boundaries in stitched meshes to prevent merging of "
                        "unrelated boundaries");
  params.addParam<bool>(
      "merge_boundaries_with_same_name",
      true,
      "If the input meshes have boundaries with the same name (but different IDs), merge them");
  params.addParam<bool>(
      "subdomain_remapping",
      true,
      "Treat input subdomain names as primary, preserving them and remapping IDs as needed");
  params.addParam<bool>(
      "verbose_stitching", false, "Whether mesh stitching should have verbose output.");
  params.addParam<bool>(
      "require_boundaries_fully_stitch",
      true,
      "If true, an error if occurs if after the stitch operation, any element "
      "does not have a neighbor across the stitch boundary. This should be disabled if you know "
      "that part of a boundary in a stitch boundary pair will not be stitched in that step.");
  params.addParam<bool>(
      "enforce_all_nodes_match_on_boundaries",
      false,
      "If true, an error occurs if not all nodes on the specified boundaries have matching "
      "positions, and this overrides the value of 'merge_boundary_nodes_all_or_nothing'.");
  params.addParam<bool>("merge_boundary_nodes_all_or_nothing",
                        false,
                        "If true, and 'enforce_all_nodes_match_on_boundaries' is false, then when "
                        "not all nodes on the specified boundaries have matching positions, no "
                        "nodes get merged, and no error occurs.");
  params.addClassDescription(
      "Allows multiple mesh files to be stitched together to form a single mesh.");

  // This is used for compatibility with TestSubGenerators
  params.addPrivateParam<bool>("_check_inputs", true);
  return params;
}

StitchMeshGenerator::StitchMeshGenerator(const InputParameters & parameters)
  : StitchMeshGeneratorBase(parameters),
    _mesh_ptrs(getMeshes("inputs")),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _prevent_boundary_ids_overlap(getParam<bool>("prevent_boundary_ids_overlap")),
    _merge_boundaries_with_same_name(getParam<bool>("merge_boundaries_with_same_name")),
    _require_boundaries_fully_stitch(getParam<bool>("require_boundaries_fully_stitch"))
{
  // Check inputs
  if (_input_names.size() - 1 != _stitch_boundaries_pairs.size() && getParam<bool>("_check_inputs"))
    paramError("stitch_boundaries_pairs",
               "Can only stitch one pair of boundary per pair of mesh. We have '" +
                   std::to_string(_input_names.size()) +
                   "' meshes specified (=" + std::to_string(_input_names.size() - 1) +
                   " pairs) and " + std::to_string(_stitch_boundaries_pairs.size()) +
                   " pairs of boundaries specified");
  for (const auto & pair : _stitch_boundaries_pairs)
    if (pair.size() != 2)
      paramError("stitch_boundaries_pairs",
                 "Stitched boundary pair '" + Moose::stringify(pair) +
                     "' is not of length 2, but of length: " + std::to_string(pair.size()));
}

std::unique_ptr<MeshBase>
StitchMeshGenerator::generate()
{
  // We put the first mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh = dynamic_pointer_cast<UnstructuredMesh>(*_mesh_ptrs[0]);
  if (!mesh) // This should never happen until libMesh implements on-the-fly-Elem mesh types
    mooseError("StitchMeshGenerator is only implemented for unstructured meshes");

  // Reserve spaces for the other meshes (no need to store the first one another time)
  std::vector<std::unique_ptr<UnstructuredMesh>> meshes(_mesh_ptrs.size() - 1);

  // Read in all of the other meshes
  for (MooseIndex(_mesh_ptrs) i = 1; i < _mesh_ptrs.size(); ++i)
    meshes[i - 1] = dynamic_pointer_cast<UnstructuredMesh>(*_mesh_ptrs[i]);

  // Stitch all the meshes to the first one
  for (MooseIndex(meshes) i = 0; i < meshes.size(); i++)
  {
    const auto boundary_pair = _stitch_boundaries_pairs[i];

    // Check the input boundaries
    const auto first = getBoundaryIdToStitch(
        *mesh,
        (_input_names.size() ? _input_names[0] : "(unknown)") +
            ((i == 0) ? "" : (" (stitched with " + std::to_string(i) + " previous meshes)")),
        boundary_pair[0]);
    auto second = getBoundaryIdToStitch(*meshes[i], name(), boundary_pair[1]);

    const bool use_binary_search = (_algorithm == "BINARY");

    // Meshes must be prepared to get the global boundary ids
    if (!mesh->is_prepared())
      mesh->prepare_for_use();
    if (!meshes[i]->is_prepared())
      meshes[i]->prepare_for_use();

    // Avoid chaotic boundary merging due to overlapping ids in meshes by simply renumbering the
    // boundaries in the meshes we are going to stitch onto the base mesh
    // This is only done if there is an overlap
    if (_prevent_boundary_ids_overlap)
    {
      const auto & base_mesh_bids = mesh->get_boundary_info().get_global_boundary_ids();
      const auto stitched_mesh_bids = meshes[i]->get_boundary_info().get_global_boundary_ids();

      // Check for an overlap
      bool overlap_found = false;
      for (const auto & bid : stitched_mesh_bids)
        if (base_mesh_bids.count(bid))
          overlap_found = true;

      // If there is any overlap, renumber all of the boundary IDs of the secondary mesh starting
      // at an index past the max of either mesh
      if (overlap_found)
      {
        const auto max_boundary_id =
            std::max(*base_mesh_bids.rbegin(), *stitched_mesh_bids.rbegin());
        BoundaryID new_index = 1;
        for (const auto bid : stitched_mesh_bids)
        {
          const auto new_bid = max_boundary_id + (new_index++);
          meshes[i]->get_boundary_info().renumber_id(bid, new_bid);
          if (bid == second)
            second = new_bid;
        }
      }
    }
    else
    {
      // If we don't have renumbering, we can get into situations where the same boundary ID is
      // associated with different boundary names. In this case when we stitch, we assign the
      // boundary name of the first mesh to the ID everywhere on the domain. We throw a warning
      // here to warn the user if this is the case.
      const auto & base_mesh_bids = mesh->get_boundary_info().get_global_boundary_ids();
      const auto & other_mesh_bids = meshes[i]->get_boundary_info().get_global_boundary_ids();

      // We check if the same ID is present with different names
      std::set<boundary_id_type> bd_id_intersection;
      std::set_intersection(base_mesh_bids.begin(),
                            base_mesh_bids.end(),
                            other_mesh_bids.begin(),
                            other_mesh_bids.end(),
                            std::inserter(bd_id_intersection, bd_id_intersection.begin()));

      for (const auto bid : bd_id_intersection)
      {
        const auto & sideset_name_on_first_mesh = mesh->get_boundary_info().get_sideset_name(bid);
        const auto & sideset_name_on_second_mesh =
            meshes[i]->get_boundary_info().get_sideset_name(bid);

        if (sideset_name_on_first_mesh != sideset_name_on_second_mesh)
          mooseWarning(
              "Boundary ID ",
              bid,
              " corresponds to different boundary names on the input meshes! On the first "
              "mesh it corresponds to `",
              sideset_name_on_first_mesh,
              "` while on the second mesh it corresponds to `",
              sideset_name_on_second_mesh,
              "`. The final mesh will replace boundary `",
              sideset_name_on_second_mesh,
              "` with `",
              sideset_name_on_first_mesh,
              "`. To avoid this situation, use the `prevent_boundary_ids_overlap` parameter!");
      }
    }

    // Save BCTuples for possibly checking stitch success
    const auto & boundary_info = mesh->get_boundary_info();
    const auto prestitch_active_side_list = boundary_info.build_active_side_list();

    mesh->stitch_meshes(*meshes[i],
                        first,
                        second,
                        getParam<Real>("stitching_hmin_tolerance_factor"),
                        _clear_stitched_boundary_ids,
                        getParam<bool>("verbose_stitching"),
                        use_binary_search,
                        getParam<bool>("enforce_all_nodes_match_on_boundaries"),
                        getParam<bool>("merge_boundary_nodes_all_or_nothing"),
                        getParam<bool>("subdomain_remapping"));

    // Check that stitching was successful: each face on the first boundary must have a neighbor
    if (_require_boundaries_fully_stitch)
      checkFullBoundaryHasNeighbor(*mesh, first, prestitch_active_side_list, _input_names[i + 1]);

    if (_merge_boundaries_with_same_name)
      MooseMeshUtils::mergeBoundaryIDsWithSameName(*mesh);
  }

  mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
StitchMeshGenerator::checkFullBoundaryHasNeighbor(
    const MeshBase & mesh,
    const BoundaryID boundary_id,
    const std::vector<libMesh::BoundaryInfo::BCTuple> & active_side_list,
    const MeshGeneratorName & mg_name) const
{
  if (!MooseMeshUtils::boundaryIsFullyInternal(mesh, boundary_id, active_side_list))
  {
    const auto & boundary_info = mesh.get_boundary_info();
    const auto & boundary_name = boundary_info.get_sideset_name(boundary_id);

    std::stringstream ss;
    ss << "After stitching mesh '" << mg_name << "', boundary '" << boundary_name
       << "' (id = " << boundary_id
       << ") is not fully internal: one or more elements on this boundary does not have a "
          "neighbor, which means that either (A) part of one or both of the boundaries in the "
          "provided boundary pair were not intended to be stitched, or (B) a stitch failure has "
          "occurred. If (A), set 'require_boundaries_fully_stitch' to false. If (B), reasons for "
          "stitch failure may include: (1) nodes along the stitch boundary do not "
          "have matching positions, (2) faces to be stitched together do not match. To determine "
          "which is the reason, try setting 'enforce_all_nodes_match_on_boundaries' to true. If "
          "the associated error is triggered, then (1) is the reason; else (2) is the reason.";
    mooseError(ss.str());
  }
}
