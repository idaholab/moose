//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvancedExtruderGenerator.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/boundary_info.h"
#include "libmesh/function_base.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_prism21.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/face_tri7.h"
#include "libmesh/libmesh_logging.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/remote_elem.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/point.h"

#include <numeric>

registerMooseObject("MooseApp", AdvancedExtruderGenerator);
registerMooseObjectRenamed("MooseApp",
                           FancyExtruderGenerator,
                           "02/18/2023 24:00",
                           AdvancedExtruderGenerator);

InputParameters
AdvancedExtruderGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh to extrude");

  params.addClassDescription(
      "Extrudes a 1D mesh into 2D, or a 2D mesh into 3D, can have a variable height for each "
      "elevation, variable number of layers within each elevation, variable growth factors of "
      "axial element sizes within each elevation and remap subdomain_ids, boundary_ids and element "
      "extra integers within each elevation as well as interface boundaries between neighboring "
      "elevation layers.");

  params.addRequiredParam<std::vector<Real>>("heights", "The height of each elevation");

  params.addRangeCheckedParam<std::vector<Real>>(
      "biases", "biases>0.0", "The axial growth factor used for mesh biasing for each elevation.");

  params.addRequiredParam<std::vector<unsigned int>>(
      "num_layers", "The number of layers for each elevation - must be num_elevations in length!");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
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

  params.addRequiredParam<Point>(
      "direction",
      "A vector that points in the direction to extrude (note, this will be "
      "normalized internally - so don't worry about it here)");

  params.addParam<boundary_id_type>(
      "top_boundary",
      "The boundary ID to set on the top boundary.  If omitted one will be generated.");

  params.addParam<boundary_id_type>(
      "bottom_boundary",
      "The boundary ID to set on the bottom boundary.  If omitted one will be generated.");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "upward_boundary_source_blocks", "Block ids used to generate upward interface boundaries.");

  params.addParam<std::vector<std::vector<boundary_id_type>>>("upward_boundary_ids",
                                                              "Upward interface boundary ids.");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "downward_boundary_source_blocks",
      "Block ids used to generate downward interface boundaries.");

  params.addParam<std::vector<std::vector<boundary_id_type>>>("downward_boundary_ids",
                                                              "Downward interface boundary ids.");
  params.addParamNamesToGroup(
      "top_boundary bottom_boundary upward_boundary_source_blocks upward_boundary_ids "
      "downward_boundary_source_blocks downward_boundary_ids",
      "Boundary Assignment");
  params.addParamNamesToGroup(
      "subdomain_swaps boundary_swaps elem_integer_names_to_swap elem_integers_swaps", "ID Swap");
  params.addParam<Real>("twist_pitch",
                        0,
                        "Pitch for helicoidal extrusion around an axis going through the origin "
                        "following the direction vector");
  return params;
}

AdvancedExtruderGenerator::AdvancedExtruderGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _heights(getParam<std::vector<Real>>("heights")),
    _biases(isParamValid("biases") ? getParam<std::vector<Real>>("biases")
                                   : std::vector<Real>(_heights.size(), 1.0)),
    _num_layers(getParam<std::vector<unsigned int>>("num_layers")),
    _subdomain_swaps(getParam<std::vector<std::vector<subdomain_id_type>>>("subdomain_swaps")),
    _boundary_swaps(getParam<std::vector<std::vector<boundary_id_type>>>("boundary_swaps")),
    _elem_integer_names_to_swap(getParam<std::vector<std::string>>("elem_integer_names_to_swap")),
    _elem_integers_swaps(
        getParam<std::vector<std::vector<std::vector<dof_id_type>>>>("elem_integers_swaps")),
    _direction(getParam<Point>("direction")),
    _has_top_boundary(isParamValid("top_boundary")),
    _top_boundary(isParamValid("top_boundary") ? getParam<boundary_id_type>("top_boundary") : 0),
    _has_bottom_boundary(isParamValid("bottom_boundary")),
    _bottom_boundary(isParamValid("bottom_boundary") ? getParam<boundary_id_type>("bottom_boundary")
                                                     : 0),
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
    _twist_pitch(getParam<Real>("twist_pitch"))
{
  if (!_direction.norm())
    paramError("direction", "Must have some length!");

  // Normalize it
  _direction /= _direction.norm();

  const auto num_elevations = _heights.size();

  if (_num_layers.size() != num_elevations)
    paramError("heights", "The length of 'heights' and 'num_layers' must be the same in ", name());

  if (_subdomain_swaps.size() && (_subdomain_swaps.size() != num_elevations))
    paramError("subdomain_swaps",
               "If specified, 'subdomain_swaps' (" + std::to_string(_subdomain_swaps.size()) +
                   ") must be the same length as 'heights' (" + std::to_string(num_elevations) +
                   ") in ",
               name());

  try
  {
    MooseMeshUtils::idSwapParametersProcessor(
        name(), "subdomain_swaps", _subdomain_swaps, _subdomain_swap_pairs);
  }
  catch (const MooseException & e)
  {
    paramError("subdomain_swaps", e.what());
  }

  if (_boundary_swaps.size() && (_boundary_swaps.size() != num_elevations))
    paramError("boundary_swaps",
               "If specified, 'boundary_swaps' (" + std::to_string(_boundary_swaps.size()) +
                   ") must be the same length as 'heights' (" + std::to_string(num_elevations) +
                   ") in ",
               name());

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

  if (_upward_boundary_source_blocks.size() != _upward_boundary_ids.size() ||
      _upward_boundary_ids.size() != num_elevations)
    paramError("upward_boundary_ids",
               "This parameter must have the same length (" +
                   std::to_string(_upward_boundary_ids.size()) +
                   ") as upward_boundary_source_blocks (" +
                   std::to_string(_upward_boundary_source_blocks.size()) + ") and heights (" +
                   std::to_string(num_elevations) + ")");
  for (unsigned int i = 0; i < _upward_boundary_source_blocks.size(); i++)
    if (_upward_boundary_source_blocks[i].size() != _upward_boundary_ids[i].size())
      paramError("upward_boundary_ids",
                 "Every element of this parameter must have the same length as the corresponding "
                 "element of upward_boundary_source_blocks.");

  if (_downward_boundary_source_blocks.size() != _downward_boundary_ids.size() ||
      _downward_boundary_ids.size() != num_elevations)
    paramError("downward_boundary_ids",
               "This parameter must have the same length (" +
                   std::to_string(_downward_boundary_ids.size()) +
                   ") as downward_boundary_source_blocks (" +
                   std::to_string(_downward_boundary_source_blocks.size()) + ") and heights (" +
                   std::to_string(num_elevations) + ")");
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

  auto mesh = buildMeshBaseObject();
  mesh->set_mesh_dimension(_input->mesh_dimension() + 1);

  // Check if the element integer names are existent in the input mesh.
  for (unsigned int i = 0; i < _elem_integer_names_to_swap.size(); i++)
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
  const auto & input_subdomain_map = _input->get_subdomain_name_map();
  const auto & input_sideset_map = _input->get_boundary_info().get_sideset_name_map();
  const auto & input_nodeset_map = _input->get_boundary_info().get_nodeset_name_map();

  // Check that the swaps source blocks are present in the mesh
  for (const auto & swap : _subdomain_swaps)
    for (const auto i : index_range(swap))
      if (i % 2 == 0 && !MooseMeshUtils::hasSubdomainID(*_input, swap[i]))
        paramError("subdomain_swaps", "The block '", swap[i], "' was not found within the mesh");

  // Check that the swaps source boundaries are present in the mesh
  for (const auto & swap : _boundary_swaps)
    for (const auto i : index_range(swap))
      if (i % 2 == 0 && !MooseMeshUtils::hasBoundaryID(*_input, swap[i]))
        paramError("boundary_swaps", "The boundary '", swap[i], "' was not found within the mesh");

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

  std::unique_ptr<MeshBase> input = std::move(_input);

  // If we're using a distributed mesh... then make sure we don't have any remote elements hanging
  // around
  if (!input->is_serial())
    mesh->delete_remote_elements();

  unsigned int total_num_layers = std::accumulate(_num_layers.begin(), _num_layers.end(), 0);

  auto total_num_elevations = _heights.size();

  dof_id_type orig_elem = input->n_elem();
  dof_id_type orig_nodes = input->n_nodes();

#ifdef LIBMESH_ENABLE_UNIQUE_ID
  unique_id_type orig_unique_ids = input->parallel_max_unique_id();
#endif

  unsigned int order = 1;

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  const BoundaryInfo & input_boundary_info = input->get_boundary_info();

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

  Point old_distance;
  Point current_distance;

  // Create translated layers of nodes in the direction of extrusion
  for (const auto & node : input->node_ptr_range())
  {
    unsigned int current_node_layer = 0;

    old_distance.zero();

    // e is the elevation layer ordering
    for (unsigned int e = 0; e < total_num_elevations; e++)
    {
      auto num_layers = _num_layers[e];

      auto height = _heights[e];

      auto bias = _biases[e];

      // k is the element layer ordering within each elevation layer
      for (unsigned int k = 0; k < order * num_layers + (e == 0 ? 1 : 0); ++k)
      {
        // For the first layer we don't need to move
        if (e == 0 && k == 0)
          current_distance.zero();
        else
        {
          // Shift the previous position by a certain fraction of 'height' along the extrusion
          // direction to get the new position.
          auto layer_index = (k - (e == 0 ? 1 : 0)) / order + 1;

          const auto step_size = MooseUtils::absoluteFuzzyEqual(bias, 1.0)
                                     ? height / (Real)num_layers / (Real)order
                                     : height * std::pow(bias, (Real)(layer_index - 1)) *
                                           (1.0 - bias) /
                                           (1.0 - std::pow(bias, (Real)(num_layers))) / (Real)order;

          current_distance = old_distance + _direction * step_size;

          // Handle helicoidal extrusion
          if (!MooseUtils::absoluteFuzzyEqual(_twist_pitch, 0.))
          {
            // twist 1 should be 'normal' to the extruded shape
            RealVectorValue twist1 = _direction.cross(*node);
            // This happens for any node on the helicoidal extrusion axis
            if (!MooseUtils::absoluteFuzzyEqual(twist1.norm(), .0))
              twist1 /= twist1.norm();
            const RealVectorValue twist2 = twist1.cross(_direction);

            auto twist = (cos(2. * libMesh::pi * layer_index * step_size / _twist_pitch) -
                          cos(2. * libMesh::pi * (layer_index - 1) * step_size / _twist_pitch)) *
                             twist2 +
                         (sin(2. * libMesh::pi * layer_index * step_size / _twist_pitch) -
                          sin(2. * libMesh::pi * (layer_index - 1) * step_size / _twist_pitch)) *
                             twist1;
            twist *= std::sqrt(node->norm_sq() + libMesh::Utility::pow<2>(_direction * (*node)));
            current_distance += twist;
          }
        }

        Node * new_node = mesh->add_point(*node + current_distance,
                                          node->id() + (current_node_layer * orig_nodes),
                                          node->processor_id());

#ifdef LIBMESH_ENABLE_UNIQUE_ID
        // Let's give the base of the extruded mesh the same
        // unique_ids as the source mesh, in case anyone finds that
        // a useful map to preserve.
        const unique_id_type uid = (current_node_layer == 0)
                                       ? node->unique_id()
                                       : orig_unique_ids +
                                             (current_node_layer - 1) * (orig_nodes + orig_elem) +
                                             node->id();

        new_node->set_unique_id(uid);
#endif

        input_boundary_info.boundary_ids(node, ids_to_copy);
        if (_boundary_swap_pairs.empty())
          boundary_info.add_node(new_node, ids_to_copy);
        else
          for (const auto & id_to_copy : ids_to_copy)
            boundary_info.add_node(new_node,
                                   _boundary_swap_pairs[e].count(id_to_copy)
                                       ? _boundary_swap_pairs[e][id_to_copy]
                                       : id_to_copy);

        old_distance = current_distance;
        current_node_layer++;
      }
    }
  }

  const std::set<boundary_id_type> & side_ids = input_boundary_info.get_side_boundary_ids();

  boundary_id_type next_side_id =
      side_ids.empty() ? 0 : cast_int<boundary_id_type>(*side_ids.rbegin() + 1);

  // side_ids may not include ids from remote elements, in which case
  // some processors may have underestimated the next_side_id; let's
  // fix that.
  input->comm().max(next_side_id);

  for (const auto & elem : input->element_ptr_range())
  {
    const ElemType etype = elem->type();

    // build_extrusion currently only works on coarse meshes
    libmesh_assert(!elem->parent());

    unsigned int current_layer = 0;

    for (unsigned int e = 0; e != total_num_elevations; e++)
    {
      auto num_layers = _num_layers[e];

      for (unsigned int k = 0; k != num_layers; ++k)
      {
        std::unique_ptr<Elem> new_elem;
        bool is_flipped(false);
        switch (etype)
        {
          case EDGE2:
          {
            new_elem = std::make_unique<Quad4>();
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer + 1) * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer + 1) * orig_nodes));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));

            break;
          }
          case EDGE3:
          {
            new_elem = std::make_unique<Quad9>();
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(4) =
                mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(5) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(6) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(7) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(8) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes));

            if (elem->neighbor_ptr(0) == remote_elem)
              new_elem->set_neighbor(3, const_cast<RemoteElem *>(remote_elem));
            if (elem->neighbor_ptr(1) == remote_elem)
              new_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));

            break;
          }
          case TRI3:
          {
            new_elem = std::make_unique<Prism6>();
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer + 1) * orig_nodes));
            new_elem->set_node(4) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer + 1) * orig_nodes));
            new_elem->set_node(5) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer + 1) * orig_nodes));

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
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(4) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(5) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(6) =
                mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(7) =
                mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(8) =
                mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(9) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(10) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(11) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(12) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(13) =
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(14) =
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(15) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(16) =
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(17) =
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 1) * orig_nodes));

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
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(4) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(5) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(6) =
                mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(7) =
                mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(8) =
                mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(9) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(10) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(11) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(12) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(13) =
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(14) =
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(15) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(16) =
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(17) =
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(18) =
                mesh->node_ptr(elem->node_ptr(6)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(19) =
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(20) =
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 1) * orig_nodes));

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
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(3)->id() + (current_layer * orig_nodes));
            new_elem->set_node(4) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer + 1) * orig_nodes));
            new_elem->set_node(5) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer + 1) * orig_nodes));
            new_elem->set_node(6) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer + 1) * orig_nodes));
            new_elem->set_node(7) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((current_layer + 1) * orig_nodes));

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
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(4) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(5) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(6) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(7) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(8) =
                mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(9) =
                mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(10) =
                mesh->node_ptr(elem->node_ptr(6)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(11) =
                mesh->node_ptr(elem->node_ptr(7)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(12) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(13) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(14) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(15) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(16) =
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(17) =
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(18) =
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(19) =
                mesh->node_ptr(elem->node_ptr(7)->id() + ((2 * current_layer + 2) * orig_nodes));

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
            new_elem->set_node(0) =
                mesh->node_ptr(elem->node_ptr(0)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(1) =
                mesh->node_ptr(elem->node_ptr(1)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(2) =
                mesh->node_ptr(elem->node_ptr(2)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(3) =
                mesh->node_ptr(elem->node_ptr(3)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(4) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(5) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(6) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(7) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(8) =
                mesh->node_ptr(elem->node_ptr(4)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(9) =
                mesh->node_ptr(elem->node_ptr(5)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(10) =
                mesh->node_ptr(elem->node_ptr(6)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(11) =
                mesh->node_ptr(elem->node_ptr(7)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(12) =
                mesh->node_ptr(elem->node_ptr(0)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(13) =
                mesh->node_ptr(elem->node_ptr(1)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(14) =
                mesh->node_ptr(elem->node_ptr(2)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(15) =
                mesh->node_ptr(elem->node_ptr(3)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(16) =
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(17) =
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(18) =
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(19) =
                mesh->node_ptr(elem->node_ptr(7)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(20) =
                mesh->node_ptr(elem->node_ptr(8)->id() + (2 * current_layer * orig_nodes));
            new_elem->set_node(21) =
                mesh->node_ptr(elem->node_ptr(4)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(22) =
                mesh->node_ptr(elem->node_ptr(5)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(23) =
                mesh->node_ptr(elem->node_ptr(6)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(24) =
                mesh->node_ptr(elem->node_ptr(7)->id() + ((2 * current_layer + 1) * orig_nodes));
            new_elem->set_node(25) =
                mesh->node_ptr(elem->node_ptr(8)->id() + ((2 * current_layer + 2) * orig_nodes));
            new_elem->set_node(26) =
                mesh->node_ptr(elem->node_ptr(8)->id() + ((2 * current_layer + 1) * orig_nodes));

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
                                             (current_layer - 1) * (orig_nodes + orig_elem) +
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
            boundary_info.add_side(added_elem, is_flipped ? top_id : 0, _bottom_boundary);
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
            boundary_info.add_side(added_elem, is_flipped ? 0 : top_id, _top_boundary);
          else
            boundary_info.add_side(
                added_elem, is_flipped ? 0 : top_id, cast_int<boundary_id_type>(next_side_id + 1));
        }

        current_layer++;
      }
    }
  }

#ifdef LIBMESH_ENABLE_UNIQUE_ID
  // Update the value of next_unique_id based on newly created nodes and elements
  // Note: Number of element layers is one less than number of node layers
  unsigned int total_new_node_layers = total_num_layers * order;
  unsigned int new_unique_ids = orig_unique_ids + (total_new_node_layers - 1) * orig_elem +
                                total_new_node_layers * orig_nodes;
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

  mesh->set_isnt_prepared();
  // Creating the layered meshes creates a lot of leftover nodes, notably in the boundary_info,
  // which will crash both paraview and trigger exodiff. Best to be safe.
  if (extruding_quad_eights)
    mesh->prepare_for_use();

  return mesh;
}
