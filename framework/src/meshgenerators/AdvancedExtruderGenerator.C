//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvancedExtruderGenerator.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"
#include "MathUtils.h"
#include "GeometryUtils.h"

#include "libmesh/boundary_info.h"
#include "libmesh/function_base.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_prism21.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/cell_c0polyhedron.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/face_tri7.h"
#include "libmesh/face_c0polygon.h"
#include "libmesh/libmesh_logging.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/remote_elem.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/point.h"

#include <numeric>
#include <cmath>

registerMooseObject("MooseApp", AdvancedExtruderGenerator);
registerMooseObjectRenamed("MooseApp",
                           FancyExtruderGenerator,
                           "02/18/2023 24:00",
                           AdvancedExtruderGenerator);

InputParameters
AdvancedExtruderGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "Extrudes a 1D mesh into 2D, or a 2D mesh into 3D, and supports a variable height for each "
      "elevation, variable number of layers within each elevation, variable growth factors of "
      "axial element sizes within each elevation and remap subdomain_ids, boundary_ids and element "
      "extra integers within each elevation as well as interface boundaries between neighboring "
      "elevation layers, as well as following a 1D curve and modifying the radial (normal to "
      "the extrusion axis) extent of the geometry.");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh to extrude");

  // User input of extrusion heights / axial discretization
  params.addParam<std::vector<Real>>("heights", {}, "The height of each elevation");
  params.addRangeCheckedParam<std::vector<Real>>(
      "biases", "biases>0.0", "The axial growth factor used for mesh biasing for each elevation.");
  params.addParam<std::vector<unsigned int>>(
      "num_layers",
      {},
      "The number of layers for each elevation - must be num_elevations in length!");

  // Swaps on every height
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "subdomain_swaps",
      {},
      "For each row, every two entries are interpreted as a pair of "
      "'from' and 'to' to remap the subdomains for that elevation");
  params.addParam<std::vector<std::vector<boundary_id_type>>>(
      "boundary_swaps",
      {},
      "For each row, every two entries are interpreted as a pair of "
      "'from' and 'to' to remap the boundaries for that elevation");
  params.addParam<std::vector<std::string>>(
      "elem_integer_names_to_swap",
      {},
      "Array of element extra integer names that need to be swapped during extrusion.");
  params.addParam<std::vector<std::vector<std::vector<dof_id_type>>>>(
      "elem_integers_swaps",
      {},
      "For each row, every two entries are interpreted as a pair of 'from' and 'to' to remap the "
      "element extra integer for that elevation. If multiple element extra integers need to be "
      "swapped, the enties are stacked based on the order provided in "
      "'elem_integer_names_to_swap' to form the third dimension.");

  // Direction parameter
  params.addParam<Point>("direction",
                         "A vector that points in the direction to extrude (note, this will be "
                         "normalized internally - so don't worry about it here)");
  params.addParam<MeshGeneratorName>(
      "extrusion_curve",
      "Name of the mesh generator providing the line mesh curve to be extruded along. The "
      "extrusion path follows the node order in the line mesh");
  params.addParam<RealVectorValue>(
      "start_extrusion_direction",
      "A vector that points in the starting direction for extruding along a curve. This vector "
      "should be the tangent vector at the FIRST node of the extrusion curve. Vector will be "
      "normalized in code, so don't worry about it here.");
  params.addParam<RealVectorValue>(
      "end_extrusion_direction",
      "A vector that points in the ending direction for extruding along a curve. This vector "
      "should be the tangent vector at the LAST node of the extrusion curve. Vector will be "
      "normalized in code, so don't worry about it here.");

  // Boundaries and interfaces
  params.addParam<BoundaryName>(
      "top_boundary",
      "The boundary name to set on the top boundary. If omitted an ID will be generated.");
  params.addParam<BoundaryName>(
      "bottom_boundary",
      "The boundary name to set on the bottom boundary. If omitted an ID will be generated.");
  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "upward_boundary_source_blocks", "Block ids used to generate upward interface boundaries.");
  params.addParam<std::vector<std::vector<boundary_id_type>>>("upward_boundary_ids",
                                                              "Upward interface boundary ids.");
  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "downward_boundary_source_blocks",
      "Block ids used to generate downward interface boundaries.");
  params.addParam<std::vector<std::vector<boundary_id_type>>>("downward_boundary_ids",
                                                              "Downward interface boundary ids.");

  // Radial transformations
  params.addParam<Real>("twist_pitch",
                        0,
                        "Pitch for helicoidal extrusion around an axis going through the origin "
                        "following the direction vector");
  params.addParam<Real>("start_radial_extent",
                        0,
                        "If modifying the radial extent of the extruded geometry, radial "
                        "extent at the beginning of the extrusion process. This can be "
                        "specified manually to avoid computing it from the surface, which can "
                        "be undesirable if the starting shape is approximated by polygonazition");
  params.addParam<Real>("end_radial_extent",
                        0,
                        "If modifying the radial extent of the extruded geometry, final radial "
                        "extent to reach at the end of the extrusion process");
  MooseEnum radial_growth_methods("linear cubic", "linear");
  params.addParam<MooseEnum>("radial_growth_method",
                             radial_growth_methods,
                             "Functional form to change radius while extruding along curve.");
  params.addParam<Real>("start_radial_growth_rate", 1., "Starting rate of radial expansion.");
  params.addParam<Real>("end_radial_growth_rate", 1., "Ending rate of radial expansion.");

  params.addParamNamesToGroup(
      "top_boundary bottom_boundary upward_boundary_source_blocks upward_boundary_ids "
      "downward_boundary_source_blocks downward_boundary_ids",
      "Boundary Assignment");
  params.addParamNamesToGroup(
      "subdomain_swaps boundary_swaps elem_integer_names_to_swap elem_integers_swaps", "ID Swap");
  params.addParamNamesToGroup("extrusion_curve start_extrusion_direction end_extrusion_direction",
                              "Extrusion along curve");
  params.addParamNamesToGroup("twist_pitch end_radial_extent radial_growth_method "
                              "start_radial_growth_rate end_radial_growth_rate",
                              "Radial transformation");

  return params;
}

AdvancedExtruderGenerator::AdvancedExtruderGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _heights(getParam<std::vector<Real>>("heights")),
    _biases(isParamValid("biases") ? getParam<std::vector<Real>>("biases")
                                   : std::vector<Real>(_heights.size(), 1.0)),
    _num_layers(getParam<std::vector<unsigned int>>("num_layers")),
    _subdomain_swaps(getParam<std::vector<std::vector<SubdomainName>>>("subdomain_swaps")),
    _boundary_swaps(getParam<std::vector<std::vector<boundary_id_type>>>("boundary_swaps")),
    _elem_integer_names_to_swap(getParam<std::vector<std::string>>("elem_integer_names_to_swap")),
    _elem_integers_swaps(
        getParam<std::vector<std::vector<std::vector<dof_id_type>>>>("elem_integers_swaps")),
    _direction(isParamValid("direction") ? getParam<Point>("direction") : Point(0, 0, 0)),
    _extrusion_curve(getMesh("extrusion_curve", true)),
    _start_extrusion_direction(isParamValid("start_extrusion_direction")
                                   ? getParam<RealVectorValue>("start_extrusion_direction").unit()
                                   : Point(0, 0, 0)),
    _end_extrusion_direction(isParamValid("end_extrusion_direction")
                                 ? getParam<RealVectorValue>("end_extrusion_direction").unit()
                                 : Point(0, 0, 0)),
    _extrude_along_curve(isParamValid("extrusion_curve")),
    _has_top_boundary(isParamValid("top_boundary")),
    _top_boundary(isParamValid("top_boundary") ? getParam<BoundaryName>("top_boundary") : "0"),
    _has_bottom_boundary(isParamValid("bottom_boundary")),
    _bottom_boundary(isParamValid("bottom_boundary") ? getParam<BoundaryName>("bottom_boundary")
                                                     : "0"),
    _upward_boundary_source_blocks(
        isParamValid("upward_boundary_source_blocks")
            ? getParam<std::vector<std::vector<subdomain_id_type>>>("upward_boundary_source_blocks")
            : std::vector<std::vector<subdomain_id_type>>(_heights.size(),
                                                          std::vector<subdomain_id_type>())),
    _upward_boundary_ids(
        isParamValid("upward_boundary_ids")
            ? getParam<std::vector<std::vector<boundary_id_type>>>("upward_boundary_ids")
            : std::vector<std::vector<boundary_id_type>>(_heights.size(),
                                                         std::vector<boundary_id_type>())),
    _downward_boundary_source_blocks(isParamValid("downward_boundary_source_blocks")
                                         ? getParam<std::vector<std::vector<subdomain_id_type>>>(
                                               "downward_boundary_source_blocks")
                                         : std::vector<std::vector<subdomain_id_type>>(
                                               _heights.size(), std::vector<subdomain_id_type>())),
    _downward_boundary_ids(
        isParamValid("downward_boundary_ids")
            ? getParam<std::vector<std::vector<boundary_id_type>>>("downward_boundary_ids")
            : std::vector<std::vector<boundary_id_type>>(_heights.size(),
                                                         std::vector<boundary_id_type>())),
    _twist_pitch(getParam<Real>("twist_pitch")),
    _end_radial_extent(getParam<Real>("end_radial_extent")),
    _radial_expansion_method(getParam<MooseEnum>("radial_growth_method")),
    _start_radial_growth_rate(getParam<Real>("start_radial_growth_rate")),
    _end_radial_growth_rate(getParam<Real>("end_radial_growth_rate"))
{
  if (_extrude_along_curve)
  {
    if (isParamSetByUser("heights"))
      paramError("heights", "heights cannot be set if extruding along curve!");
    if (isParamValid("biases"))
      paramError("biases", "biases cannot be set if extruding along curve!");
    if (isParamSetByUser("num_layers"))
      paramError("num_layers", "num_layers cannot be set if extruding along curve!");
    if (isParamValid("direction"))
      paramError("direction", "direction cannot be set if extruding along curve!");
  }
  else
  {
    if (!_direction.norm())
      paramError("direction", "Must have some length!");
    _direction /= _direction.norm();
  }

  unsigned int num_elevations;
  if (_extrude_along_curve)
    num_elevations = 1;
  else
  {
    num_elevations = _heights.size();
    if (_num_layers.size() != num_elevations)
      paramError(
          "heights", "The length of 'heights' and 'num_layers' must be the same in ", name());
  }

  if (_subdomain_swaps.size() && (_subdomain_swaps.size() != num_elevations))
  {
    if (_extrude_along_curve)
      paramError("subdomain_swaps",
                 "If specified, 'subdomain_swaps' (" + std::to_string(_subdomain_swaps.size()) +
                     ") must be equal to 1 when extruding along a curve.");
    else
    {
      paramError("subdomain_swaps",
                 "If specified, 'subdomain_swaps' (" + std::to_string(_subdomain_swaps.size()) +
                     ") must be the same length as 'heights' (" + std::to_string(num_elevations) +
                     ") in ",
                 name());
    }
  }

  if (_boundary_swaps.size() && (_boundary_swaps.size() != num_elevations))
  {
    if (_extrude_along_curve)
    {
      paramError("boundary_swaps",
                 "If specified, 'boundary_swaps' (" + std::to_string(_boundary_swaps.size()) +
                     ") must be the same length as 'heights' (" + std::to_string(num_elevations) +
                     ") in ",
                 name());
    }
    else
    {
      paramError("boundary_swaps",
                 "If specified, 'boundary_swaps' (" + std::to_string(_boundary_swaps.size()) +
                     ") must be equal to 1 when extruding along a curve.");
    }
  }

  try
  {
    MooseMeshUtils::idSwapParametersProcessor(
        name(), "boundary_swaps", _boundary_swaps, _boundary_swap_pairs);
  }
  catch (const MooseException & e)
  {
    paramError("boundary_swaps", e.what());
  }

  if (_elem_integers_swaps.size() &&
      _elem_integers_swaps.size() != _elem_integer_names_to_swap.size())
    paramError("elem_integers_swaps",
               "If specified, 'elem_integers_swaps' must have the same length as the length of "
               "'elem_integer_names_to_swap'.");

  for (const auto & unit_elem_integers_swaps : _elem_integers_swaps)
    if (unit_elem_integers_swaps.size() != num_elevations)
      paramError("elem_integers_swaps",
                 "If specified, each element of 'elem_integers_swaps' must have the same length as "
                 "the length of 'heights'.");

  try
  {
    MooseMeshUtils::extraElemIntegerSwapParametersProcessor(name(),
                                                            num_elevations,
                                                            _elem_integer_names_to_swap.size(),
                                                            _elem_integers_swaps,
                                                            _elem_integers_swap_pairs);
  }
  catch (const MooseException & e)
  {
    paramError("elem_integers_swaps", e.what());
  }

  if (!_extrude_along_curve)
  {
    bool has_negative_entry = false;
    bool has_positive_entry = false;
    for (const auto & h : _heights)
    {
      if (h > 0.0)
        has_positive_entry = true;
      else
        has_negative_entry = true;
    }

    if (has_negative_entry && has_positive_entry)
      paramError("heights", "Cannot have both positive and negative heights!");
    if (_biases.size() != _heights.size())
      paramError("biases", "Size of this parameter, if provided, must be the same as heights.");
  }

  if ((_upward_boundary_source_blocks.size() || _upward_boundary_ids.size()) &&
      (_upward_boundary_source_blocks.size() != _upward_boundary_ids.size() ||
       _upward_boundary_ids.size() != num_elevations))
  {
    if (_extrude_along_curve)
      paramError(
          "upward_boundary_ids",
          "This parameter must have the same length (" +
              std::to_string(_upward_boundary_ids.size()) + ") as upward_boundary_source_blocks (" +
              std::to_string(_upward_boundary_source_blocks.size()) + ") and num_elevations (" +
              std::to_string(num_elevations) +
              "). Note that the number of heights is set to 1 when extruding along a curve.");
    else
    {
      paramError("upward_boundary_ids",
                 "This parameter must have the same length (" +
                     std::to_string(_upward_boundary_ids.size()) +
                     ") as upward_boundary_source_blocks (" +
                     std::to_string(_upward_boundary_source_blocks.size()) + ") and heights (" +
                     std::to_string(num_elevations) + ")");
    }
  }

  for (unsigned int i = 0; i < _upward_boundary_source_blocks.size(); i++)
    if (_upward_boundary_source_blocks[i].size() != _upward_boundary_ids[i].size())
      paramError("upward_boundary_ids",
                 "Every element of this parameter must have the same length as the corresponding "
                 "element of upward_boundary_source_blocks.");

  if ((_downward_boundary_source_blocks.size() || _downward_boundary_ids.size()) &&
      (_downward_boundary_source_blocks.size() != _downward_boundary_ids.size() ||
       _downward_boundary_ids.size() != num_elevations))
  {
    if (_extrude_along_curve)
      paramError(
          "downward_boundary_ids",
          "This parameter must have the same length (" +
              std::to_string(_downward_boundary_ids.size()) +
              ") as downward_boundary_source_blocks (" +
              std::to_string(_downward_boundary_source_blocks.size()) + ") and (" +
              std::to_string(num_elevations) +
              "). Note that the number of heights is set to 1 when extruding along a curve.");
    else
    {
      paramError("downward_boundary_ids",
                 "This parameter must have the same length (" +
                     std::to_string(_downward_boundary_ids.size()) +
                     ") as downward_boundary_source_blocks (" +
                     std::to_string(_downward_boundary_source_blocks.size()) + ") and heights (" +
                     std::to_string(num_elevations) + ")");
    }
  }
  for (unsigned int i = 0; i < _downward_boundary_source_blocks.size(); i++)
    if (_downward_boundary_source_blocks[i].size() != _downward_boundary_ids[i].size())
      paramError("downward_boundary_ids",
                 "Every element of this parameter must have the same length as the corresponding "
                 "element of downward_boundary_source_blocks.");
}

std::unique_ptr<MeshBase>
AdvancedExtruderGenerator::generate()
{
  // Note: bulk of this code originally from libmesh mesh_modification.C
  // Original copyright: Copyright (C) 2002-2019 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner
  // Original license is LGPL so it can be used here.

  auto mesh = buildMeshBaseObject(_input->mesh_dimension() + 1);
  mesh->set_mesh_dimension(_input->mesh_dimension() + 1);

  // Check if the element integer names are existent in the input mesh.
  for (const auto i : make_range(_elem_integer_names_to_swap.size()))
    if (_input->has_elem_integer(_elem_integer_names_to_swap[i]))
      _elem_integer_indices_to_swap.push_back(
          _input->get_elem_integer_index(_elem_integer_names_to_swap[i]));
    else
      paramError("elem_integer_names_to_swap",
                 "Element ",
                 i + 1,
                 " of 'elem_integer_names_to_swap' in is not a valid extra element integer of the "
                 "input mesh.");

  // prepare for transferring extra element integers from original mesh to the extruded mesh.
  const unsigned int num_extra_elem_integers = _input->n_elem_integers();
  std::vector<std::string> id_names;

  for (unsigned int i = 0; i < num_extra_elem_integers; i++)
  {
    id_names.push_back(_input->get_elem_integer_name(i));
    if (!mesh->has_elem_integer(id_names[i]))
      mesh->add_elem_integer(id_names[i]);
  }

  // retrieve subdomain/sideset/nodeset name maps
  if (!_input->preparation().has_cached_elem_data)
    _input->cache_elem_data();
  const auto & input_subdomain_map = _input->get_subdomain_name_map();
  const auto & input_sideset_map = _input->get_boundary_info().get_sideset_name_map();
  const auto & input_nodeset_map = _input->get_boundary_info().get_nodeset_name_map();

  // Check that the swaps source blocks are present in the mesh
  for (const auto & swap : _subdomain_swaps)
    for (const auto i : index_range(swap))
      if (i % 2 == 0 && !MooseMeshUtils::hasSubdomainName(*_input, swap[i]))
        paramError("subdomain_swaps",
                   "The block '",
                   swap[i],
                   "' was not found within the mesh.\nBlocks in the mesh: " +
                       Moose::stringify(MooseMeshUtils::getAllSubdomainNamesAndIDs(*_input)));

  // Check that the swaps source boundaries are present in the mesh
  for (const auto & swap : _boundary_swaps)
    for (const auto i : index_range(swap))
      if (i % 2 == 0 && !MooseMeshUtils::hasBoundaryID(*_input, swap[i]))
        paramError("boundary_swaps",
                   "The boundary '",
                   swap[i],
                   "' was not found within the mesh.\nBoundaries in the mesh: " +
                       Moose::stringify(MooseMeshUtils::getAllBoundaryNamesAndIDs(*_input)));

  // Check that the source blocks for layer top/bottom boundaries exist in the mesh
  for (const auto & layer_vec : _upward_boundary_source_blocks)
    for (const auto bid : layer_vec)
      if (!MooseMeshUtils::hasSubdomainID(*_input, bid))
        paramError(
            "upward_boundary_source_blocks", "The block '", bid, "' was not found within the mesh");
  for (const auto & layer_vec : _downward_boundary_source_blocks)
    for (const auto bid : layer_vec)
      if (!MooseMeshUtils::hasSubdomainID(*_input, bid))
        paramError("downward_boundary_source_blocks",
                   "The block '",
                   bid,
                   "' was not found within the mesh");

  // Process the subdomain swap parameters to work with IDs
  if (_subdomain_swaps.size())
  {
    std::vector<std::vector<subdomain_id_type>> subdomain_swaps_ids(_subdomain_swaps.size());
    for (const auto i : index_range(_subdomain_swaps))
    {
      subdomain_swaps_ids[i].resize(_subdomain_swaps[i].size());
      // Source blocks exist in the mesh, we already checked
      // Target blocks might now, we'll need to add them
      for (const auto j : make_range(_subdomain_swaps[i].size() / 2))
      {
        subdomain_swaps_ids[i][2 * j] =
            MooseMeshUtils::getSubdomainID(_subdomain_swaps[i][2 * j], *_input);
        auto target_block_id =
            MooseMeshUtils::getSubdomainID(_subdomain_swaps[i][2 * j + 1], *_input);
        if (target_block_id == Moose::INVALID_BLOCK_ID)
        {
          target_block_id = MooseMeshUtils::getNextFreeSubdomainID(*_input);
          // Add the subdomain names for any newly created subdomain
          mesh->subdomain_name(target_block_id) = _subdomain_swaps[i][2 * j + 1];
        }
        subdomain_swaps_ids[i][2 * j + 1] = target_block_id;
      }
    }
    try
    {
      MooseMeshUtils::idSwapParametersProcessor(
          name(), "subdomain_swaps", subdomain_swaps_ids, _subdomain_swap_pairs);
    }
    catch (const MooseException & e)
    {
      paramError("subdomain_swaps", e.what());
    }
  }

  // Move the meshes as requested by the mesh generator system
  std::unique_ptr<MeshBase> input = std::move(_input);
  std::unique_ptr<MeshBase> extrusion_curve;
  if (_extrude_along_curve)
    extrusion_curve = std::move(_extrusion_curve);

  // We must serialize the curve to know how to extrude across all ranks
  std::unique_ptr<libMesh::MeshSerializer> serializer;
  if (_extrude_along_curve)
    serializer = std::make_unique<libMesh::MeshSerializer>(*extrusion_curve);

  // If we're using a distributed mesh... then make sure we don't have any remote elements hanging
  // around
  if (!input->is_serial())
  {
    input->delete_remote_elements();
    // This should be a no-op, and yet, it's needed for two THM tests
    mesh->delete_remote_elements();
  }

  if (input->n_nodes() != input->max_node_id())
    input->renumber_nodes_and_elements();

  if (input->n_nodes() != input->max_node_id())
    mooseError(
        "You must allow renumbering, because the extruded mesh should be contiguously numbered. "
        "Alternatively, you can use a separate mesh generator (MeshRepairGenerator with the "
        "renumber_contiguously parameter for example) to renumber the nodes contiguously.");

  unsigned int total_num_layers;
  unsigned int total_num_elevations;
  if (!_extrude_along_curve)
  {
    total_num_layers = std::accumulate(_num_layers.begin(), _num_layers.end(), 0);
    total_num_elevations = _heights.size();
  }
  else
  {
    total_num_layers = extrusion_curve->n_elem();
    total_num_elevations = 1;
  }

  dof_id_type orig_elem = input->n_elem();
  dof_id_type orig_nodes = input->n_nodes();

#ifdef LIBMESH_ENABLE_UNIQUE_ID
  unique_id_type orig_unique_ids = input->parallel_max_unique_id();
  bool has_poly_midnodes = false;
#endif

  bool has_polygons = false;
  unsigned int order = 1;

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  const BoundaryInfo & input_boundary_info = input->get_boundary_info();

  // Determine boundary IDs for the new user provided boundary names
  std::vector<BoundaryName> new_boundary_names;
  if (_has_bottom_boundary)
    new_boundary_names.push_back(_bottom_boundary);
  if (_has_top_boundary)
    new_boundary_names.push_back(_top_boundary);
  std::vector<boundary_id_type> new_boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*input, new_boundary_names, true);
  const auto user_bottom_boundary_id =
      _has_bottom_boundary ? new_boundary_ids.front() : libMesh::BoundaryInfo::invalid_id;
  const auto user_top_boundary_id =
      _has_top_boundary ? new_boundary_ids.back() : libMesh::BoundaryInfo::invalid_id;

  // We know a priori how many elements we'll need
  mesh->reserve_elem(total_num_layers * orig_elem);

  // Look for higher order elements which introduce an extra layer
  std::set<ElemType> higher_orders = {EDGE3, EDGE4, TRI6, TRI7, QUAD8, QUAD9};
  bool extruding_quad_eights = false;
  std::vector<ElemType> types;
  MeshTools::elem_types(*input, types);
  for (const auto elem_type : types)
  {
    if (higher_orders.count(elem_type))
      order = 2;
    if (elem_type == QUAD8)
      extruding_quad_eights = true;
  }
  mesh->comm().max(order);
  mesh->comm().max(extruding_quad_eights);

  // Reserve for the max number possibly needed
  mesh->reserve_nodes((order * total_num_layers + 1) * orig_nodes);

  // Container to catch the boundary IDs handed back by the BoundaryInfo object
  std::vector<boundary_id_type> ids_to_copy;

  // We need to compute the distance to the axis for radial expansions
  Real start_radial_extent = std::numeric_limits<Real>::max();
  Point reference_point;
  if (_extrude_along_curve)
    reference_point = *(extrusion_curve->node_ptr(0));
  else if (!MooseUtils::absoluteFuzzyEqual(_twist_pitch, 0.))
    // Backwards compatibility: we were twisting along an axis going through the origin
    reference_point = Point(0, 0, 0);
  else
    reference_point = MooseMeshUtils::meshCentroidCalculator(*input);

  if (_end_radial_extent)
  {
    // the center is at the curve if we are extruding along a curve, and the centroid of the mesh
    // otherwise
    RealVectorValue reference_direction;
    if (_extrude_along_curve)
      reference_direction =
          _start_extrusion_direction.norm_sq() > 0
              ? _start_extrusion_direction
              : RealVectorValue(
                    (*(extrusion_curve->node_ptr(1)) - *(extrusion_curve->node_ptr(0))).unit());
    else
      reference_direction = _direction;

    // now we measure the initial radial extent
    if (!isParamValid("start_radial_extent"))
      start_radial_extent =
          MooseMeshUtils::computeMaxDistanceToAxis(*input, reference_point, reference_direction);
    else
      start_radial_extent = getParam<Real>("start_radial_extent");
  }

  // Compute the total extrusion distance along the axis
  Real total_extrusion_distance_at_axis = 0;
  if (_extrude_along_curve)
  {
    if (!extrusion_curve->is_prepared())
      extrusion_curve->prepare_for_use();
    total_extrusion_distance_at_axis = MeshTools::volume(*extrusion_curve);
  }
  else
    for (const auto h : _heights)
      total_extrusion_distance_at_axis += h;
  mooseAssert(total_extrusion_distance_at_axis > 0, "Should not be 0");

  // Create translated layers of nodes in the direction of extrusion
  for (const auto & node : input->node_ptr_range())
  {
    unsigned int current_node_layer = 0;
    Point orig_node_to_previous;
    Point orig_node_to_current;
    Real sum_step_sizes = 0.;
    Real sum_step_sizes_at_axis = 0.;
    // Initial distance from the node to the extrusion axis
    Real start_node_radius = (*node - reference_point).norm();

    // e is the elevation layer ordering
    for (const auto e : make_range(total_num_elevations))
    {
      unsigned int num_layers = libMesh::invalid_uint;
      Real height = std::numeric_limits<Real>::max(), bias = std::numeric_limits<Real>::max();
      if (_extrude_along_curve)
      {
        num_layers = extrusion_curve->n_elem();
      }
      else
      {
        num_layers = _num_layers[e];
        height = _heights[e];
        bias = _biases[e];
      }

      // In first layer we also add the base nodes, hence the "plus one"
      unsigned int num_heights_at_elevation = order * num_layers + (e == 0 ? 1 : 0);
      // Keep track of the plane normal to the extrusion
      RealVectorValue prev_intersecting_plane_normal_vec = _start_extrusion_direction;

      // k is the element layer ordering within each elevation layer
      for (const auto k : make_range(num_heights_at_elevation))
      {
        // Compute 'orig_node_to_current', the update vector
        // For the first layer we don't need to move
        if (e == 0 && k == 0)
          orig_node_to_current.zero();
        else
        {
          // Shift the previous position by a certain fraction of 'height' along the extrusion
          // direction to get the new position.

          // Compute orig_node_to_current before any transformation
          Real step_size = 0;
          Real step_size_at_axis = 0;
          if (_extrude_along_curve)
          {
            // current point in extrusion curve
            const Node * P_current = extrusion_curve->node_ptr(k);
            // previous point in extrusion curve
            const Node * P_prev = extrusion_curve->node_ptr(k - 1);

            // Quantities for the previous position of the extruded node
            const auto old_node = orig_node_to_previous + *node;
            RealVectorValue b_vec = old_node - *P_prev;
            Real node_distance_to_curve = b_vec.norm();

            // Node does not lie exactly on the extrusion curve we are following
            if (node_distance_to_curve > libMesh::TOLERANCE)
            {
              // normal vector to the plane to extrude the point into
              RealVectorValue intersecting_plane_normal_vec;

              // Select the extrusion direction based on the curve or the user parameters
              if (k == 1)
              {
                const auto P_next = extrusion_curve->node_ptr(k + 1);
                intersecting_plane_normal_vec =
                    0.5 * (_start_extrusion_direction + (*P_next - *P_current).unit());
              }
              else if (k < order * num_layers - 1)
              {
                const auto P_next = extrusion_curve->node_ptr(k + 1);
                // this approximates the derivative at the spline point
                intersecting_plane_normal_vec = *P_next - *P_prev;
              }
              else
              {
                intersecting_plane_normal_vec = (_end_extrusion_direction.norm_sq() > 0)
                                                    ? _end_extrusion_direction
                                                    : RealVectorValue(*P_current - *P_prev);
              }
              intersecting_plane_normal_vec /= intersecting_plane_normal_vec.norm();

              Point new_node_point;
              // If the extrusion direction and the previous plane normal are aligned,
              // we can't define a rotation axis. We simply translate the points
              if (MooseUtils::absoluteFuzzyEqual(
                      prev_intersecting_plane_normal_vec.cross(intersecting_plane_normal_vec)
                          .norm_sq(),
                      0))
                new_node_point = old_node + *P_current - *P_prev;
              // We use a rotation from the previous extrusion plane normal to the current one
              // We have tried in the past:
              // - assuming (P_prev, P_current, old_node, current_node) are coplanar
              // - assuming (P_prev, P_current) and (old_node, current_node) are parallel
              // Both result in a slight deformation of the shape during extrusion.
              else
              {
                // v = axis * sin(theta), unnormalized — avoids dividing by sin_th
                const auto v =
                    prev_intersecting_plane_normal_vec.cross(intersecting_plane_normal_vec);
                const Real cos_th =
                    prev_intersecting_plane_normal_vec * intersecting_plane_normal_vec;

                mooseAssert(cos_th > -1.0 + 1e-10,
                            "Degenerate 180-degree rotation between consecutive plane normals");

                // Rodrigues formula, vector form
                // R(x) = x*cos_th + (v cross x) + v*(v dot x)/(1+cos_th)
                const auto new_v =
                    cos_th * b_vec + v.cross(b_vec) + v * (v * b_vec) / (1. + cos_th);
                mooseAssert(new_v * b_vec >= 0, "Should be positive");

                mooseAssert(MooseUtils::absoluteFuzzyEqual(new_v.norm(), b_vec.norm()),
                            "Radial extent be conserved");
                new_node_point = *P_current + new_v;
              }

              orig_node_to_current = new_node_point - *node;

              step_size = (orig_node_to_current - orig_node_to_previous).norm();
              step_size_at_axis = (*P_current - *P_prev).norm();
              prev_intersecting_plane_normal_vec = intersecting_plane_normal_vec;
            }
            // Point is on the axis of the line
            else
            {
              _direction = *P_current - *P_prev;
              _direction /= _direction.norm();
              mooseAssert(std::abs(_direction.norm() - 1.0) < libMesh::TOLERANCE,
                          "Norm of direction vector is not 1!");

              // Calculate step size.
              // Note: orig_node_to_previous + *node is the vector description of the
              // previously-created node
              step_size = ((*P_current - (orig_node_to_previous + *node)) * _direction);
              step_size_at_axis = step_size;
              orig_node_to_current = orig_node_to_previous + _direction * step_size;
            }
          }
          // Extruding in a fixed direction (not along a curve)
          else
          {
            // Divide by the order to avoid applying the bias to the nodes within a higher order
            // element
            auto layer_index = (k - (e == 0 ? 1 : 0)) / order + 1;
            step_size = MooseUtils::absoluteFuzzyEqual(bias, 1.0)
                            ? height / (Real)num_layers / (Real)order
                            : height * std::pow(bias, (Real)(layer_index - 1)) * (1.0 - bias) /
                                  (1.0 - std::pow(bias, (Real)(num_layers))) / (Real)order;
            step_size_at_axis = step_size;
            orig_node_to_current =
                orig_node_to_previous +
                _direction * step_size; // update distance from starting node to new node
          }

          // Keep track of the cumulative step size as an extrusion axis coordinate
          sum_step_sizes += step_size;
          sum_step_sizes_at_axis += step_size_at_axis;

          // Handle radial expansion. No need to perform expansion at the centerline
          if (_end_radial_extent && start_node_radius > libMesh::TOLERANCE)
          {
            // How far along we are in the extrusion, measured at the axis (=curve when extruding
            // along a curve)
            Real tm1 =
                (sum_step_sizes_at_axis - step_size_at_axis) / total_extrusion_distance_at_axis;
            Real t = sum_step_sizes_at_axis / total_extrusion_distance_at_axis;

            // Direction of radial expansion.
            RealVectorValue node_to_extrusion_axis;
            if (_extrude_along_curve)
              node_to_extrusion_axis =
                  *node + orig_node_to_current - *(extrusion_curve->node_ptr(k));
            // The radial expansion has to be performed in the rotated frame
            else if (!MooseUtils::absoluteFuzzyEqual(_twist_pitch, 0.))
              node_to_extrusion_axis =
                  *node + orig_node_to_current - (reference_point + _direction * sum_step_sizes);
            // when extruding in a straight line with no twisting, we can use the original radial
            // direction
            else
              node_to_extrusion_axis = *node - reference_point;

            // Fraction of the total starting radius the node lives in
            const auto radius_scaling = start_node_radius / start_radial_extent;
            // Calculate weighting for expansion
            const auto radial_ratio_m1 = AdvancedExtruderGenerator::radialExpansionRatio(tm1);
            const auto radial_ratio = AdvancedExtruderGenerator::radialExpansionRatio(t);
            mooseAssert(radial_ratio > 0, "Should be positive");

            // Compute change in step
            orig_node_to_current += (start_radial_extent * (radial_ratio_m1 - radial_ratio) +
                                     (radial_ratio - radial_ratio_m1) * _end_radial_extent) *
                                    radius_scaling * node_to_extrusion_axis.unit();
          }

          // Handle helicoidal extrusion
          if (!MooseUtils::absoluteFuzzyEqual(_twist_pitch, 0.))
          {
            // twist 1 should be 'normal' to the extruded shape
            RealVectorValue twist1 = _direction.cross(*node);
            // This happens for any node on the helicoidal extrusion axis
            if (!MooseUtils::absoluteFuzzyEqual(twist1.norm(), .0))
              twist1 /= twist1.norm();
            const RealVectorValue twist2 = twist1.cross(_direction);

            auto twist =
                (cos(2. * libMesh::pi * current_node_layer * step_size / _twist_pitch) -
                 cos(2. * libMesh::pi * (current_node_layer - 1) * step_size / _twist_pitch)) *
                    twist2 +
                (sin(2. * libMesh::pi * current_node_layer * step_size / _twist_pitch) -
                 sin(2. * libMesh::pi * (current_node_layer - 1) * step_size / _twist_pitch)) *
                    twist1;

            // If we normalize twist, we must multiply by 2 * sin(libMesh::pi * step_size /
            // _twist_pitch) if (!MooseUtils::absoluteFuzzyEqual(twist.norm(), .0))
            //   twist /= twist.norm();

            // Get a point on the extrusion axis (around which we currently twist the geometry)
            // at the local elevation height to be able to compute the distance to the axis
            Point extrusion_axis_at_elevation;
            Point prev_extrusion_axis_at_elevation;
            if (_extrude_along_curve)
            {
              extrusion_axis_at_elevation = *extrusion_curve->node_ptr(k);
              prev_extrusion_axis_at_elevation = *extrusion_curve->node_ptr(k - 1);
            }
            else
            {
              // We can't use old and current step updates because they have been "twisted"
              extrusion_axis_at_elevation = reference_point + _direction * sum_step_sizes;
              prev_extrusion_axis_at_elevation =
                  reference_point + _direction * (sum_step_sizes - step_size);
            }

            // Scale with how far the node is from the extrusion axis, before twisting
            twist *= (*node + orig_node_to_current - extrusion_axis_at_elevation).norm();

            // No need to twist or expand the point on the axis
            if (!MooseUtils::absoluteFuzzyEqual(twist1.norm(), .0))
              orig_node_to_current += twist;
          }
        }

        // Add the new node to the mesh
        Node * new_node = mesh->add_point(*node + orig_node_to_current,
                                          node->id() + (current_node_layer * orig_nodes),
                                          node->processor_id());

#ifdef LIBMESH_ENABLE_UNIQUE_ID
        // Let's give the base of the extruded mesh the same
        // unique_ids as the source mesh, in case anyone finds that
        // a useful map to preserve.
        const unique_id_type uid = (current_node_layer == 0)
                                       ? node->unique_id()
                                       : orig_unique_ids +
                                             (current_node_layer - 1) * (orig_elem + orig_nodes) +
                                             node->id();
        new_node->set_unique_id(uid);
#endif

        // Add the new node to the extruded boundaries
        input_boundary_info.boundary_ids(node, ids_to_copy);
        if (_boundary_swap_pairs.empty())
          boundary_info.add_node(new_node, ids_to_copy);
        else
          for (const auto & id_to_copy : ids_to_copy)
          {
            boundary_info.add_node(new_node,
                                   _boundary_swap_pairs[e].count(id_to_copy)
                                       ? _boundary_swap_pairs[e][id_to_copy]
                                       : id_to_copy);
          }

        orig_node_to_previous = orig_node_to_current;
        current_node_layer++;
      }
    }
  }

  const auto & side_ids = input_boundary_info.get_side_boundary_ids();

  boundary_id_type next_side_id =
      side_ids.empty() ? 0 : cast_int<boundary_id_type>(*side_ids.rbegin() + 1);

  // side_ids may not include ids from remote elements, in which case
  // some processors may have underestimated the next_side_id; let's
  // fix that.
  input->comm().max(next_side_id);

  // Map to keep track of polygon sides of polygonal prisms
  std::map<std::array<unsigned int, 3>, std::shared_ptr<libMesh::Polygon>> poly_extruded_sides;

  // Build the extruded elements
  for (const auto & elem : input->element_ptr_range())
  {
    const ElemType etype = elem->type();

    // build_extrusion currently only works on coarse meshes
    libmesh_assert(!elem->parent());

    unsigned int current_layer = 0;

    for (unsigned int e = 0; e != total_num_elevations; e++)
    {
      auto num_layers = !_extrude_along_curve ? _num_layers[e] : extrusion_curve->n_nodes() - 1;

      for (unsigned int k = 0; k != num_layers; ++k)
      {
        std::unique_ptr<Elem> new_elem;
        bool is_flipped(false);
        switch (etype)
        {
          case EDGE2:
          {
            new_elem = std::make_unique<Quad4>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                2, mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                3, mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer + 1) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));

            break;
          }
          case EDGE3:
          {
            new_elem = std::make_unique<Quad9>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                2,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                3,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                4, mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                5,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                6,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                7,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                8,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));

            break;
          }
          case TRI3:
          {
            new_elem = std::make_unique<Prism6>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                2, mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                3, mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                4, mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                5, mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer + 1) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(2, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(2) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));

            if (new_elem->volume() < 0.0)
            {
              MooseMeshUtils::swapNodesInElem(*new_elem, 0, 3);
              MooseMeshUtils::swapNodesInElem(*new_elem, 1, 4);
              MooseMeshUtils::swapNodesInElem(*new_elem, 2, 5);
              is_flipped = true;
            }

            break;
          }
          case TRI6:
          {
            new_elem = std::make_unique<Prism18>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                2, mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                3,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                4,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                5,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                6, mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                7, mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                8, mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                9,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                10,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                11,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                12,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                13,
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                14,
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                15,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                16,
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                17,
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 1) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(2, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(2) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));

            if (new_elem->volume() < 0.0)
            {
              MooseMeshUtils::swapNodesInElem(*new_elem, 0, 3);
              MooseMeshUtils::swapNodesInElem(*new_elem, 1, 4);
              MooseMeshUtils::swapNodesInElem(*new_elem, 2, 5);
              MooseMeshUtils::swapNodesInElem(*new_elem, 6, 12);
              MooseMeshUtils::swapNodesInElem(*new_elem, 7, 13);
              MooseMeshUtils::swapNodesInElem(*new_elem, 8, 14);
              is_flipped = true;
            }

            break;
          }
          case TRI7:
          {
            new_elem = std::make_unique<Prism21>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                2, mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                3,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                4,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                5,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                6, mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                7, mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                8, mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                9,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                10,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                11,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                12,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                13,
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                14,
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                15,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                16,
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                17,
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                18, mesh->node_ptr(elem->node_ptr(6)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                19,
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                20,
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 1) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(2, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(2) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));

            if (new_elem->volume() < 0.0)
            {
              MooseMeshUtils::swapNodesInElem(*new_elem, 0, 3);
              MooseMeshUtils::swapNodesInElem(*new_elem, 1, 4);
              MooseMeshUtils::swapNodesInElem(*new_elem, 2, 5);
              MooseMeshUtils::swapNodesInElem(*new_elem, 6, 12);
              MooseMeshUtils::swapNodesInElem(*new_elem, 7, 13);
              MooseMeshUtils::swapNodesInElem(*new_elem, 8, 14);
              MooseMeshUtils::swapNodesInElem(*new_elem, 18, 19);
              is_flipped = true;
            }

            break;
          }
          case QUAD4:
          {
            new_elem = std::make_unique<Hex8>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                2, mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                3, mesh->node_ptr(elem->node_ptr(3)->id() + (current_layer * orig_nodes)));
            new_elem->set_node(
                4, mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                5, mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                6, mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                7, mesh->node_ptr(elem->node_ptr(3)->id() + ((current_layer + 1) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(2, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(2) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(3) == remote_elem)
              new_elem->set_neighbor(4, const_cast<RemoteElem *>(remote_elem));

            if (new_elem->volume() < 0.0)
            {
              MooseMeshUtils::swapNodesInElem(*new_elem, 0, 4);
              MooseMeshUtils::swapNodesInElem(*new_elem, 1, 5);
              MooseMeshUtils::swapNodesInElem(*new_elem, 2, 6);
              MooseMeshUtils::swapNodesInElem(*new_elem, 3, 7);
              is_flipped = true;
            }

            break;
          }
          case QUAD8:
          {
            new_elem = std::make_unique<Hex20>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                2, mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                3, mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                4,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                5,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                6,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                7,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                8, mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                9, mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                10, mesh->node_ptr(elem->node_ptr(6)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                11, mesh->node_ptr(elem->node_ptr(7)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                12,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                13,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                14,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                15,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                16,
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                17,
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                18,
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                19,
                mesh->node_ptr(elem->node_ptr(7)->id() + ((2 * current_layer + 2) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(2, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(2) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(3) == remote_elem)
              new_elem->set_neighbor(4, const_cast<RemoteElem *>(remote_elem));

            if (new_elem->volume() < 0.0)
            {
              MooseMeshUtils::swapNodesInElem(*new_elem, 0, 4);
              MooseMeshUtils::swapNodesInElem(*new_elem, 1, 5);
              MooseMeshUtils::swapNodesInElem(*new_elem, 2, 6);
              MooseMeshUtils::swapNodesInElem(*new_elem, 3, 7);
              MooseMeshUtils::swapNodesInElem(*new_elem, 8, 16);
              MooseMeshUtils::swapNodesInElem(*new_elem, 9, 17);
              MooseMeshUtils::swapNodesInElem(*new_elem, 10, 18);
              MooseMeshUtils::swapNodesInElem(*new_elem, 11, 19);
              is_flipped = true;
            }

            break;
          }
          case QUAD9:
          {
            new_elem = std::make_unique<Hex27>();
            new_elem->set_node(
                0, mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                1, mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                2, mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                3, mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                4,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                5,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                6,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                7,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                8, mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                9, mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                10, mesh->node_ptr(elem->node_ptr(6)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                11, mesh->node_ptr(elem->node_ptr(7)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                12,
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                13,
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                14,
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                15,
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                16,
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                17,
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                18,
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                19,
                mesh->node_ptr(elem->node_ptr(7)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                20, mesh->node_ptr(elem->node_ptr(8)->id() + (2 * current_layer * orig_nodes)));
            new_elem->set_node(
                21,
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                22,
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                23,
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                24,
                mesh->node_ptr(elem->node_ptr(7)->id() + ((2 * current_layer + 1) * orig_nodes)));
            new_elem->set_node(
                25,
                mesh->node_ptr(elem->node_ptr(8)->id() + ((2 * current_layer + 2) * orig_nodes)));
            new_elem->set_node(
                26,
                mesh->node_ptr(elem->node_ptr(8)->id() + ((2 * current_layer + 1) * orig_nodes)));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(2, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(2) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(3) == remote_elem)
              new_elem->set_neighbor(4, const_cast<RemoteElem *>(remote_elem));

            if (new_elem->volume() < 0.0)
            {
              MooseMeshUtils::swapNodesInElem(*new_elem, 0, 4);
              MooseMeshUtils::swapNodesInElem(*new_elem, 1, 5);
              MooseMeshUtils::swapNodesInElem(*new_elem, 2, 6);
              MooseMeshUtils::swapNodesInElem(*new_elem, 3, 7);
              MooseMeshUtils::swapNodesInElem(*new_elem, 8, 16);
              MooseMeshUtils::swapNodesInElem(*new_elem, 9, 17);
              MooseMeshUtils::swapNodesInElem(*new_elem, 10, 18);
              MooseMeshUtils::swapNodesInElem(*new_elem, 11, 19);
              MooseMeshUtils::swapNodesInElem(*new_elem, 20, 25);
              is_flipped = true;
            }

            break;
          }
          case libMesh::C0POLYGON:
          {
            has_polygons = true;
            const auto num_sides = elem->n_sides();
            std::vector<std::shared_ptr<libMesh::Polygon>> sides;
            sides.reserve(2 + num_sides);
            if (2 * elem->n_nodes() > libMesh::C0Polygon::max_n_nodes)
              mooseError("Too many nodes in polygons to extrude it. Max number of the prism "
                         "polyhedral nodes after extrusion: " +
                         std::to_string(libMesh::C0Polygon::max_n_nodes));
            // Make a copy of the original element to use as a side
            auto new_ptr = std::make_shared<libMesh::C0Polygon>(num_sides);
            for (const auto node_i : make_range(elem->n_nodes()))
            {
              // This one will be oriented outwards, it should be OK though, polyhedron code does
              // not mind
              new_ptr->set_node(
                  node_i,
                  mesh->node_ptr(elem->node_ptr(node_i)->id() + (current_layer * orig_nodes)));
            }
            sides.push_back(new_ptr);
            // Form the next horizontal side
            auto translated_side = std::make_shared<libMesh::C0Polygon>(num_sides);
            for (const auto node_i : make_range(elem->n_nodes()))
              translated_side->set_node(node_i,
                                        mesh->node_ptr(elem->node_ptr(node_i)->id() +
                                                       ((current_layer + 1) * orig_nodes)));
            sides.push_back(translated_side);

            // Form the vertical sides
            for (const auto side_i : make_range(num_sides))
            {
              // If the side already exists, use that
              std::array<unsigned int, 3> side_key = {
                  current_layer,
                  static_cast<unsigned int>(
                      std::min(elem->node_ptr(side_i)->id(),
                               elem->node_ptr((side_i + 1) % num_sides)->id())),
                  static_cast<unsigned int>(
                      std::max(elem->node_ptr(side_i)->id(),
                               elem->node_ptr((side_i + 1) % num_sides)->id()))};
              if (poly_extruded_sides.count(side_key))
              {
                sides.push_back(poly_extruded_sides[side_key]);
                continue;
              }

              // They are all quads, but constructor expects polygons
              auto vert_side = std::make_shared<libMesh::C0Polygon>(4);
              vert_side->set_node(
                  0, mesh->node_ptr(elem->node_ptr(side_i)->id() + (current_layer * orig_nodes)));
              vert_side->set_node(1,
                                  mesh->node_ptr(elem->node_ptr((side_i + 1) % num_sides)->id() +
                                                 (current_layer * orig_nodes)));
              vert_side->set_node(2,
                                  mesh->node_ptr(elem->node_ptr((side_i + 1) % num_sides)->id() +
                                                 ((current_layer + 1) * orig_nodes)));
              vert_side->set_node(3,
                                  mesh->node_ptr(elem->node_ptr(side_i)->id() +
                                                 ((current_layer + 1) * orig_nodes)));
              sides.push_back(vert_side);

              poly_extruded_sides.insert(std::make_pair(side_key, vert_side));
            }
            mooseAssert(sides.size() == 2 + num_sides, "Unexpected size of side vector");

            // Create the element from the sides, let libMesh figure out the orientation
            std::unique_ptr<libMesh::Node> mid_elem_node;
            new_elem = std::make_unique<libMesh::C0Polyhedron>(sides, mid_elem_node);
            if (mid_elem_node)
            {
#ifdef LIBMESH_ENABLE_UNIQUE_ID
              // Number it at the end for convenience
              // use the element ID to be able to set in parallel
              unsigned int total_new_node_layers = total_num_layers * order;
              unsigned int last_uid = orig_unique_ids + (total_new_node_layers - 1) * orig_elem +
                                      total_new_node_layers * orig_nodes + elem->unique_id();
              mid_elem_node->set_unique_id(last_uid);
              has_poly_midnodes = true;
#endif
              mesh->add_node(std::move(mid_elem_node));
            }

            break;
          }
          default:
            mooseError("Extrusion is not implemented for element type " + Moose::stringify(etype));
        }

        new_elem->set_id(elem->id() + (current_layer * orig_elem));
        new_elem->processor_id() = elem->processor_id();

#ifdef LIBMESH_ENABLE_UNIQUE_ID
        // Let's give the base of the extruded mesh the same
        // unique_ids as the source mesh, in case anyone finds that
        // a useful map to preserve.
        const unique_id_type uid = (current_layer == 0)
                                       ? elem->unique_id()
                                       : orig_unique_ids +
                                             (current_layer - 1) * (orig_elem + orig_nodes) +
                                             orig_nodes + elem->id();

        new_elem->set_unique_id(uid);
#endif

        // maintain the subdomain_id
        new_elem->subdomain_id() = elem->subdomain_id();

        // define upward boundaries
        if (k == num_layers - 1)
        {
          // Identify the side index of the new element that is part of the upward boundary
          const unsigned short top_id =
              new_elem->dim() == 3 ? cast_int<unsigned short>(elem->n_sides() + 1) : 2;
          // Assign sideset id to the side if the element belongs to a specified
          // upward_boundary_source_blocks
          if (_upward_boundary_source_blocks.size() || _upward_boundary_ids.size())
            for (unsigned int i = 0; i < _upward_boundary_source_blocks[e].size(); i++)
              if (new_elem->subdomain_id() == _upward_boundary_source_blocks[e][i])
                boundary_info.add_side(
                    new_elem.get(), is_flipped ? 0 : top_id, _upward_boundary_ids[e][i]);
        }
        // define downward boundaries
        if (k == 0)
        {
          const unsigned short top_id =
              new_elem->dim() == 3 ? cast_int<unsigned short>(elem->n_sides() + 1) : 2;
          if (_downward_boundary_source_blocks.size() || _downward_boundary_ids.size())
            for (unsigned int i = 0; i < _downward_boundary_source_blocks[e].size(); i++)
              if (new_elem->subdomain_id() == _downward_boundary_source_blocks[e][i])
                boundary_info.add_side(
                    new_elem.get(), is_flipped ? top_id : 0, _downward_boundary_ids[e][i]);
        }

        // perform subdomain swaps
        if (_subdomain_swap_pairs.size())
        {
          auto & elevation_swap_pairs = _subdomain_swap_pairs[e];

          auto new_id_it = elevation_swap_pairs.find(elem->subdomain_id());

          if (new_id_it != elevation_swap_pairs.end())
            new_elem->subdomain_id() = new_id_it->second;
        }

        Elem * added_elem = mesh->add_elem(std::move(new_elem));

        // maintain extra integers
        for (unsigned int i = 0; i < num_extra_elem_integers; i++)
          added_elem->set_extra_integer(i, elem->get_extra_integer(i));

        if (_elem_integers_swap_pairs.size())
        {
          for (unsigned int i = 0; i < _elem_integer_indices_to_swap.size(); i++)
          {
            auto & elevation_extra_swap_pairs = _elem_integers_swap_pairs[i * _heights.size() + e];

            auto new_extra_id_it = elevation_extra_swap_pairs.find(
                elem->get_extra_integer(_elem_integer_indices_to_swap[i]));

            if (new_extra_id_it != elevation_extra_swap_pairs.end())
              added_elem->set_extra_integer(_elem_integer_indices_to_swap[i],
                                            new_extra_id_it->second);
          }
        }

        // Copy any old boundary ids on all sides
        for (auto s : elem->side_index_range())
        {
          input_boundary_info.boundary_ids(elem, s, ids_to_copy);

          if (added_elem->dim() == 3)
          {
            // For 2D->3D extrusion, we give the boundary IDs
            // for side s on the old element to side s+1 on the
            // new element.  This is just a happy coincidence as
            // far as I can tell...
            if (_boundary_swap_pairs.empty())
              boundary_info.add_side(added_elem, cast_int<unsigned short>(s + 1), ids_to_copy);
            else
              for (const auto & id_to_copy : ids_to_copy)
                boundary_info.add_side(added_elem,
                                       cast_int<unsigned short>(s + 1),
                                       _boundary_swap_pairs[e].count(id_to_copy)
                                           ? _boundary_swap_pairs[e][id_to_copy]
                                           : id_to_copy);
          }
          else
          {
            // For 1D->2D extrusion, the boundary IDs map as:
            // Old elem -> New elem
            // 0        -> 3
            // 1        -> 1
            libmesh_assert_less(s, 2);
            const unsigned short sidemap[2] = {3, 1};
            if (_boundary_swap_pairs.empty())
              boundary_info.add_side(added_elem, sidemap[s], ids_to_copy);
            else
              for (const auto & id_to_copy : ids_to_copy)
                boundary_info.add_side(added_elem,
                                       sidemap[s],
                                       _boundary_swap_pairs[e].count(id_to_copy)
                                           ? _boundary_swap_pairs[e][id_to_copy]
                                           : id_to_copy);
          }
        }

        // Give new boundary ids to bottom and top
        if (current_layer == 0)
        {
          const unsigned short top_id =
              added_elem->dim() == 3 ? cast_int<unsigned short>(elem->n_sides() + 1) : 2;
          if (_has_bottom_boundary)
          {
            mooseAssert(user_bottom_boundary_id != libMesh::BoundaryInfo::invalid_id,
                        "We should have retrieved a proper boundary ID");
            boundary_info.add_side(added_elem, is_flipped ? top_id : 0, user_bottom_boundary_id);
          }
          else
            boundary_info.add_side(added_elem, is_flipped ? top_id : 0, next_side_id);
        }

        if (current_layer == total_num_layers - 1)
        {
          // For 2D->3D extrusion, the "top" ID is 1+the original
          // element's number of sides.  For 1D->2D extrusion, the
          // "top" ID is side 2.
          const unsigned short top_id =
              added_elem->dim() == 3 ? cast_int<unsigned short>(elem->n_sides() + 1) : 2;

          if (_has_top_boundary)
          {
            mooseAssert(user_top_boundary_id != libMesh::BoundaryInfo::invalid_id,
                        "We should have retrieved a proper boundary ID");
            boundary_info.add_side(added_elem, is_flipped ? 0 : top_id, user_top_boundary_id);
          }
          else
            boundary_info.add_side(
                added_elem, is_flipped ? 0 : top_id, cast_int<boundary_id_type>(next_side_id + 1));
        }

        current_layer++;
      }
    }
  }

  if (has_polygons && !input->is_serial())
    mooseError("Distributed meshes are not supported when extruding polygons at this time.");

#ifdef LIBMESH_ENABLE_UNIQUE_ID
  // Update the value of next_unique_id based on newly created nodes and elements
  // Note: Number of element layers is one less than number of node layers
  unsigned int total_new_node_layers = total_num_layers * order;
  unsigned int new_unique_ids = orig_unique_ids + (total_new_node_layers - 1) * orig_elem +
                                total_new_node_layers * orig_nodes;
  // Maximum case for the unique ids: all poly elements have a midnode
  if (has_poly_midnodes)
    new_unique_ids += orig_elem;
  mesh->set_next_unique_id(new_unique_ids);
#endif

  // Copy all the subdomain/sideset/nodeset name maps to the extruded mesh
  if (!input_subdomain_map.empty())
    mesh->set_subdomain_name_map().insert(input_subdomain_map.begin(), input_subdomain_map.end());
  if (!input_sideset_map.empty())
    mesh->get_boundary_info().set_sideset_name_map().insert(input_sideset_map.begin(),
                                                            input_sideset_map.end());
  if (!input_nodeset_map.empty())
    mesh->get_boundary_info().set_nodeset_name_map().insert(input_nodeset_map.begin(),
                                                            input_nodeset_map.end());

  if (_has_bottom_boundary)
    boundary_info.sideset_name(new_boundary_ids.front()) = new_boundary_names.front();
  if (_has_top_boundary)
    boundary_info.sideset_name(new_boundary_ids.back()) = new_boundary_names.back();

  mesh->unset_is_prepared();
  // Creating the layered meshes creates a lot of leftover nodes, notably in the boundary_info,
  // which will crash both paraview and trigger exodiff. Best to be safe.
  if (extruding_quad_eights)
    mesh->prepare_for_use();

  return mesh;
}

Real
AdvancedExtruderGenerator::radialExpansionRatio(const Real t) const
{
  // NOTE: All functions added to this method must obey the following: f(0)=0, f(1)=1 for all t in
  // [0,1].
  switch (_radial_expansion_method)
  {
    case 0:
      return t;
    // Only linear and cubic implemented
    default:
      return (-2. + _start_radial_growth_rate + _end_radial_growth_rate) * MathUtils::pow(t, 3) +
             (3. - (2. * _start_radial_growth_rate + _end_radial_growth_rate)) *
                 MathUtils::pow(t, 2) +
             _start_radial_growth_rate * t;
  }
}
