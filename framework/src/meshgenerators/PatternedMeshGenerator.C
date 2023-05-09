//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatternedMeshGenerator.h"

#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_tools.h"
#include "MooseMeshUtils.h"

registerMooseObject("MooseApp", PatternedMeshGenerator);

InputParameters
PatternedMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  params.addRangeCheckedParam<Real>(
      "x_width", 0, "x_width>=0", "The tile width in the x direction");
  params.addRangeCheckedParam<Real>(
      "y_width", 0, "y_width>=0", "The tile width in the y direction");
  params.addRangeCheckedParam<Real>(
      "z_width", 0, "z_width>=0", "The tile width in the z direction");

  // Boundaries : user has to provide id or name for each boundary

  // x boundary names
  params.addParam<BoundaryName>("left_boundary", "left", "name of the left (x) boundary");
  params.addParam<BoundaryName>("right_boundary", "right", "name of the right (x) boundary");

  // y boundary names
  params.addParam<BoundaryName>("top_boundary", "top", "name of the top (y) boundary");
  params.addParam<BoundaryName>("bottom_boundary", "bottom", "name of the bottom (y) boundary");

  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern", "A double-indexed array starting with the upper-left corner");

  params.addClassDescription("Creates a 2D mesh from a specified set of unique 'tiles' meshes and "
                             "a two-dimensional pattern.");

  return params;
}

PatternedMeshGenerator::PatternedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _mesh_ptrs(getMeshes("inputs")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _x_width(getParam<Real>("x_width")),
    _y_width(getParam<Real>("y_width")),
    _z_width(getParam<Real>("z_width"))
{
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
      if (_pattern[i][j] >= _input_names.size())
        paramError("pattern",
                   "Index " + Moose::stringify(_pattern[i][j]) +
                       " is larger than the the maximum possible index, which is determined by the "
                       "number of MeshGenerators provided in inputs");
}

std::unique_ptr<MeshBase>
PatternedMeshGenerator::generate()
{
  // Reserve spaces for all the meshes
  _meshes.reserve(_input_names.size());

  // Getting the boundaries provided by the user
  const std::vector<BoundaryName> boundary_names = {getParam<BoundaryName>("left_boundary"),
                                                    getParam<BoundaryName>("right_boundary"),
                                                    getParam<BoundaryName>("top_boundary"),
                                                    getParam<BoundaryName>("bottom_boundary")};
  const std::vector<std::string> boundary_param_names = {
      "left_boundary", "right_boundary", "top_boundary", "bottom_boundary"};

  // IDs for each input (indexed by input mesh and then left/right/top/bottom)
  std::vector<std::vector<boundary_id_type>> input_bids(
      _input_names.size(), std::vector<boundary_id_type>(4, Moose::INVALID_BOUNDARY_ID));
  bool have_common_ids = true;

  // Using a vector of vectors instead of vector of sets to preserve insertion order
  std::vector<std::vector<boundary_id_type>> input_bids_unique(_input_names.size());

  // For an error check on the number of uniquely named boundaries to stitch
  size_t set_length = 0;

  // Given a boundary name (i.e., 'left_boundary'), this will give the correct
  // index to get the correct boundary id to later use for boundary stitching
  std::map<std::string, size_t> boundary_name_to_index_map;

  // Keep track of used boundary ids to generate new, unused ones later (if needed)
  std::set<boundary_id_type> all_boundary_ids;

  // Read in all of the meshes
  _meshes.resize(_input_names.size());
  for (const auto i : index_range(_input_names))
  {
    std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs[i]);
    if (!mesh)
      paramError("inputs",
                 "The input mesh '",
                 _input_names[i],
                 "' is not a replicated mesh.\n\n",
                 type(),
                 " only works with inputs that are replicated.\n\n",
                 "Try running without distributed mesh.");
    _meshes[i] = dynamic_pointer_cast<ReplicatedMesh>(mesh);

    // List of boundary ids corresponsind to left/right/top/bottom boundary names
    const auto ids = MooseMeshUtils::getBoundaryIDs(*_meshes[i], boundary_names, false);
    mooseAssert(ids.size() == boundary_names.size(),
                "Unexpected number of ids returned for MooseMeshUtils::getBoundaryIDs");

    // Keep track of indices of first instance of each unique boundary id
    std::map<boundary_id_type, size_t> seen_bid_to_index_map;

    size_t index = 0;
    for (const auto side : make_range(4))
    {
      // Check if the boundary has been initialized
      if (ids[side] == Moose::INVALID_BOUNDARY_ID)
        paramError("inputs",
                   "The '",
                   boundary_param_names[side],
                   "' parameter with value '",
                   boundary_names[side],
                   "' does not exist in input mesh '",
                   _input_names[i],
                   "'");

      input_bids[i][side] = ids[side];

      // We only do this when i == 0 because all input meshes should have the
      // same index map. Allowing different index maps for different input
      // meshes results in undefined behaviour when stitching
      if (i == 0)
      {
        if (std::count(input_bids_unique[i].begin(), input_bids_unique[i].end(), ids[side]) == 0)
        {
          input_bids_unique[i].push_back(ids[side]);
          seen_bid_to_index_map[ids[side]] = index;
          boundary_name_to_index_map[boundary_param_names[side]] = index++;
        }
        else
          boundary_name_to_index_map[boundary_param_names[side]] = seen_bid_to_index_map[ids[side]];
      }

      else // i > 0
      {
        if (ids[side] != input_bids[i - 1][side])
          have_common_ids = false;

        if (std::count(input_bids_unique[i].begin(), input_bids_unique[i].end(), ids[side]) == 0)
          input_bids_unique[i].push_back(ids[side]);
      }
    }

    // Error check on lengths of input_bids_unique
    if (i > 0 && set_length != input_bids_unique.size())
      mooseError(
          "Input meshes have incompatible boundary ids. This can occur when input meshes have "
          "the same boundary id for multiple boundaries, but in a way that is different "
          "between the meshes. Try assigning each left/right/top/bottom to its own boundary id.");

    set_length = input_bids_unique.size();

    // List of all boundary ids used in _meshes[i] so we don't reuse any existing boundary ids.
    const auto all_ids = _meshes[i]->get_boundary_info().get_boundary_ids();

    // Keep track of used IDs so we can later find IDs that are unused across all meshes
    all_boundary_ids.insert(all_ids.begin(), all_ids.end());
  }

  // Check if the user has provided the x, y and z widths.
  // If not (their value is 0 by default), compute them
  auto bbox = MeshTools::create_bounding_box(*_meshes[0]);
  if (_x_width == 0)
    _x_width = bbox.max()(0) - bbox.min()(0);
  if (_y_width == 0)
    _y_width = bbox.max()(1) - bbox.min()(1);
  if (_z_width == 0)
    _z_width = bbox.max()(2) - bbox.min()(2);

  // stitch_bids will hold boundary ids passed to mesh stitcher
  std::vector<boundary_id_type> stitch_bids;

  if (have_common_ids) // No need to change existing boundary ids
    stitch_bids = input_bids_unique[0];

  else // Need to make boundary ids common accross all inputs
  {
    // Generate previously unused boundary ids
    for (boundary_id_type id = 0; id != Moose::INVALID_BOUNDARY_ID; ++id)
      if (!all_boundary_ids.count(id))
      {
        stitch_bids.push_back(id);
        // It is okay to only use the 0th index here, since we ensure all entries in
        // input_bids_unique have the same size through the above error check.
        if (stitch_bids.size() == input_bids_unique[0].size())
          break;
      }

    // Make all inputs have common boundary ids
    for (const auto i : index_range(_meshes))
      for (const auto side : index_range(stitch_bids))
        MeshTools::Modification::change_boundary_id(
            *_meshes[i], input_bids_unique[i][side], stitch_bids[side]);
  }

  // Data structure that holds each row
  _row_meshes.resize(_pattern.size());

  // Aliases
  const boundary_id_type &left_bid(stitch_bids[boundary_name_to_index_map["left_boundary"]]),
      right_bid(stitch_bids[boundary_name_to_index_map["right_boundary"]]),
      top_bid(stitch_bids[boundary_name_to_index_map["top_boundary"]]),
      bottom_bid(stitch_bids[boundary_name_to_index_map["bottom_boundary"]]);

  // Build each row mesh
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      Real deltax = j * _x_width, deltay = i * _y_width;

      // If this is the first cell of the row initialize the row mesh
      if (j == 0)
      {
        auto clone = _meshes[_pattern[i][j]]->clone();
        _row_meshes[i] = dynamic_pointer_cast<ReplicatedMesh>(clone);

        MeshTools::Modification::translate(*_row_meshes[i], deltax, -deltay, 0);

        continue;
      }

      ReplicatedMesh & cell_mesh = *_meshes[_pattern[i][j]];

      // Move the mesh into the right spot.  -i because we are starting at the top
      MeshTools::Modification::translate(cell_mesh, deltax, -deltay, 0);

      // Subdomain map is aggregated on each row first. This retrieves a writable reference
      auto & main_subdomain_map = _row_meshes[i]->set_subdomain_name_map();
      // Retrieve subdomain name map from the mesh to be stitched and merge into the row's
      // subdomain map
      const auto & increment_subdomain_map = cell_mesh.get_subdomain_name_map();
      mergeSubdomainNameMaps(main_subdomain_map, increment_subdomain_map);

      _row_meshes[i]->stitch_meshes(cell_mesh,
                                    right_bid,
                                    left_bid,
                                    TOLERANCE,
                                    /*clear_stitched_boundary_ids=*/true,
                                    /*verbose=*/false);

      // Undo the translation
      MeshTools::Modification::translate(cell_mesh, -deltax, deltay, 0);
    }

  // Now stitch together the rows
  // We're going to stitch them all to row 0 (which is the real mesh)
  for (MooseIndex(_pattern) i = 1; i < _pattern.size(); i++)
  {
    // Get a writeable reference subdomain-name map for the main mesh to which the other rows are
    // stitched
    auto & main_subdomain_map = _row_meshes[0]->set_subdomain_name_map();
    // Retrieve subdomain name map from the mesh to be stitched and merge into the main
    // subdomain map
    const auto & increment_subdomain_map = _row_meshes[i]->get_subdomain_name_map();
    mergeSubdomainNameMaps(main_subdomain_map, increment_subdomain_map);

    _row_meshes[0]->stitch_meshes(*_row_meshes[i],
                                  bottom_bid,
                                  top_bid,
                                  TOLERANCE,
                                  /*clear_stitched_boundary_ids=*/true,
                                  /*verbose=*/false);
  }

  // Change boundary ids back to those of meshes[0] to not surprise user
  if (!have_common_ids)
    for (const auto side : index_range(stitch_bids))
      MeshTools::Modification::change_boundary_id(
          *_row_meshes[0], stitch_bids[side], input_bids_unique[0][side]);

  _row_meshes[0]->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(_row_meshes[0]);
}

void
PatternedMeshGenerator::mergeSubdomainNameMaps(
    std::map<subdomain_id_type, std::string> & main_subdomain_map,
    const std::map<subdomain_id_type, std::string> & increment_subdomain_map)
{
  // Insert secondary subdomain map into main subdomain map
  main_subdomain_map.insert(increment_subdomain_map.begin(), increment_subdomain_map.end());
  // Check if one SubdomainName is shared by more than one subdomain ids
  std::set<SubdomainName> main_subdomain_map_name_list;
  for (auto const & id_name_pair : main_subdomain_map)
  {
    const auto name_to_insert = id_name_pair.second;
    if (main_subdomain_map_name_list.find(name_to_insert) != main_subdomain_map_name_list.end())
      paramError("inputs",
                 "Two of the input meshes contain a subdomain with the name '" + name_to_insert +
                     "' which corresponds to two conflicting subdomain ids.");
    main_subdomain_map_name_list.emplace(name_to_insert);
  }
}
