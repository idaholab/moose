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

  // boundary names in input mesh
  params.addParam<BoundaryName>(
      "left_boundary", "left", "name of the left (x) boundary in input mesh");
  params.addParam<BoundaryName>(
      "right_boundary", "right", "name of the right (x) boundary in input mesh");
  params.addParam<BoundaryName>(
      "top_boundary", "top", "name of the top (y) boundary in input mesh");
  params.addParam<BoundaryName>(
      "bottom_boundary", "bottom", "name of the bottom (y) boundary in input mesh");

  // new boundary names
  params.addParam<BoundaryName>(
      "new_left_boundary", "left", "name of the new left (x) boundary in output mesh");
  params.addParam<BoundaryName>(
      "new_right_boundary", "right", "name of the new right (x) boundary in output mesh");
  params.addParam<BoundaryName>(
      "new_top_boundary", "top", "name of the new top (y) boundary in output mesh");
  params.addParam<BoundaryName>(
      "new_bottom_boundary", "bottom", "name of the new bottom (y) boundary in output mesh");

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

  // Read in all of the meshes
  for (MooseIndex(_input_names) i = 0; i < _input_names.size(); ++i)
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
    _meshes.push_back(std::move(mesh));
  }

  // Data structure that holds each row
  _row_meshes.resize(_pattern.size());

  // Getting the boundaries provided by the user
  std::vector<BoundaryName> boundary_names = {getParam<BoundaryName>("left_boundary"),
                                              getParam<BoundaryName>("right_boundary"),
                                              getParam<BoundaryName>("top_boundary"),
                                              getParam<BoundaryName>("bottom_boundary")};

  std::vector<boundary_id_type> ids =
      MooseMeshUtils::getBoundaryIDs(*_meshes[0], boundary_names, true);

  mooseAssert(ids.size() == boundary_names.size(),
              "Unexpected number of ids returned for MooseMeshUtils::getBoundaryIDs");

  boundary_id_type left = ids[0], right = ids[1], top = ids[2], bottom = ids[3];

  // Check if all the boundaries have been initialized
  if (left == -123)
    mooseError("The left boundary has not been initialized properly.");
  if (right == -123)
    mooseError("The right boundary has not been initialized properly.");
  if (top == -123)
    mooseError("The top boundary has not been initialized properly.");
  if (bottom == -123)
    mooseError("The bottom boundary has not been initialized properly.");

  // Check if the user has provided the x, y and z widths.
  // If not (their value is 0 by default), compute them
  auto bbox = MeshTools::create_bounding_box(*_meshes[0]);
  if (_x_width == 0)
    _x_width = bbox.max()(0) - bbox.min()(0);
  if (_y_width == 0)
    _y_width = bbox.max()(1) - bbox.min()(1);
  if (_z_width == 0)
    _z_width = bbox.max()(2) - bbox.min()(2);

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
                                    right,
                                    left,
                                    TOLERANCE,
                                    /*clear_stitched_boundary_ids=*/true);

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

    _row_meshes[0]->stitch_meshes(
        *_row_meshes[i], bottom, top, TOLERANCE, /*clear_stitched_boundary_ids=*/true);
  }

  // Set outer boundary sidesets and nodesets
  BoundaryInfo & boundary_info = _row_meshes[0]->get_boundary_info();

  // Redefine boundary ids for output mesh
  boundary_names = {
      isParamValid("new_left_boundary") ? getParam<BoundaryName>("new_left_boundary")
                                        : getParam<BoundaryName>("left_boundary"),
      isParamValid("new_right_boundary") ? getParam<BoundaryName>("new_right_boundary")
                                         : getParam<BoundaryName>("right_boundary"),
      isParamValid("new_top_boundary") ? getParam<BoundaryName>("new_top_boundary")
                                       : getParam<BoundaryName>("top_boundary"),
      isParamValid("new_bottom_boundary") ? getParam<BoundaryName>("new_bottom_boundary")
                                          : getParam<BoundaryName>("bottom_boundary")};
  ids = MooseMeshUtils::getBoundaryIDs(*_row_meshes[0], boundary_names, true);

  // Remove existing outer boundary information
  for (unsigned int i = 0; i < ids.size(); i++)
    boundary_info.remove_id(ids[i]);

  // Redefine bounding box for patterned mesh and generate outer boundary
  // locations
  bbox = MeshTools::create_bounding_box(*_row_meshes[0]);
  Real min_x = bbox.min()(0);
  Real max_x = bbox.max()(0);
  Real min_y = bbox.min()(1);
  Real max_y = bbox.max()(1);

  // Set outer boundary nodes and sides
  for (const auto & elem : _row_meshes[0]->active_element_ptr_range())
  {
    const Node * const * elem_nodes = elem->get_nodes();
    unsigned int n_nodes = elem->n_nodes();
    for (unsigned int n = 0; n < n_nodes; ++n)
    {
      Point p(*elem_nodes[n]);
      // Left boundary
      if (MooseUtils::absoluteFuzzyEqual(p(0), min_x))
        boundary_info.add_node(elem_nodes[n], ids[0]);
      // Right boundary
      if (MooseUtils::absoluteFuzzyEqual(p(0), max_x))
        boundary_info.add_node(elem_nodes[n], ids[1]);
      // Top boundary
      if (MooseUtils::absoluteFuzzyEqual(p(1), max_y))
        boundary_info.add_node(elem_nodes[n], ids[2]);
      // Bottom boundary
      if (MooseUtils::absoluteFuzzyEqual(p(1), min_y))
        boundary_info.add_node(elem_nodes[n], ids[3]);
    }
    unsigned int n_sides = elem->n_sides();
    for (unsigned int i = 0; i < n_sides; ++i)
    {
      std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);
      Point p0(*curr_side->node_ptr(0));
      Point p1(*curr_side->node_ptr(1));
      if (MooseUtils::absoluteFuzzyEqual(p0(0), min_x) &&
          MooseUtils::absoluteFuzzyEqual(p1(0), min_x))
        boundary_info.add_side(elem, i, ids[0]);
      if (MooseUtils::absoluteFuzzyEqual(p0(0), max_x) &&
          MooseUtils::absoluteFuzzyEqual(p1(0), max_x))
        boundary_info.add_side(elem, i, ids[1]);
      if (MooseUtils::absoluteFuzzyEqual(p0(1), max_y) &&
          MooseUtils::absoluteFuzzyEqual(p1(1), max_y))
        boundary_info.add_side(elem, i, ids[2]);
      if (MooseUtils::absoluteFuzzyEqual(p0(1), min_y) &&
          MooseUtils::absoluteFuzzyEqual(p1(1), min_y))
        boundary_info.add_side(elem, i, ids[3]);
    }
  }

  // Set outer boundary names
  for (unsigned int i = 0; i < boundary_names.size(); i++)
  {
    boundary_info.nodeset_name(ids[i]) = boundary_names[i];
    boundary_info.sideset_name(ids[i]) = boundary_names[i];
  }

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
                 "The input meshes both contain subdomain name " + name_to_insert +
                     " that correspond to conflicting subdomain ids.");
    main_subdomain_map_name_list.emplace(name_to_insert);
  }
}
