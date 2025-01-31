//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RevolveGenerator.h"
#include "PolygonalMeshGenerationUtils.h"

#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism15.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_prism21.h"
#include "libmesh/cell_pyramid5.h"
#include "libmesh/cell_pyramid13.h"
#include "libmesh/cell_pyramid14.h"
#include "libmesh/cell_pyramid18.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/cell_tet10.h"
#include "libmesh/cell_tet14.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri7.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad9.h"
#include "libmesh/point.h"
#include "libmesh/mesh_tools.h"

using namespace libMesh;

// C++ includes
#include <cmath>

registerMooseObject("ReactorApp", RevolveGenerator);

InputParameters
RevolveGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addClassDescription("This RevolveGenerator object is designed to revolve a 1D mesh into "
                             "2D, or a 2D mesh into 3D based on an axis.");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh to revolve");

  params.addRequiredParam<Point>("axis_point", "A point on the axis of revolution");

  params.addRequiredParam<Point>("axis_direction", "The direction of the axis of revolution");

  params.addRangeCheckedParam<std::vector<Real>>(
      "revolving_angles",
      "revolving_angles<=360.0 & revolving_angles>0.0",
      "The angles delineating each azimuthal section of revolution around the axis in degrees");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "subdomain_swaps",
      {},
      "For each row, every two entries are interpreted as a pair of "
      "'from' and 'to' to remap the subdomains for that azimuthal section");

  params.addParam<std::vector<std::vector<boundary_id_type>>>(
      "boundary_swaps",
      {},
      "For each row, every two entries are interpreted as a pair of "
      "'from' and 'to' to remap the boundaries for that elevation");

  params.addParam<std::vector<std::string>>(
      "elem_integer_names_to_swap",
      {},
      "Array of element extra integer names that need to be swapped during revolving.");

  params.addParam<std::vector<std::vector<std::vector<dof_id_type>>>>(
      "elem_integers_swaps",
      {},
      "For each row, every two entries are interpreted as a pair of 'from' and 'to' to remap the "
      "element extra integer for that elevation. If multiple element extra integers need to be "
      "swapped, the enties are stacked based on the order provided in "
      "'elem_integer_names_to_swap' to form the third dimension.");

  params.addParam<boundary_id_type>(
      "start_boundary",
      "The boundary ID to set on the starting boundary for a partial revolution.");

  params.addParam<boundary_id_type>(
      "end_boundary", "The boundary ID to set on the ending boundary for partial revolving.");

  params.addParam<bool>(
      "clockwise", true, "Revolve clockwise around the axis or not (i.e., counterclockwise)");

  params.addRequiredParam<std::vector<unsigned int>>(
      "nums_azimuthal_intervals",
      "List of the numbers of azimuthal interval discretization for each azimuthal section");

  params.addParam<bool>("preserve_volumes",
                        false,
                        "Whether the volume of the revolved mesh is preserving the circular area "
                        "by modifying (expanding) the radius to account for polygonization.");

  params.addParamNamesToGroup("start_boundary end_boundary", "Boundary Assignment");
  params.addParamNamesToGroup(
      "subdomain_swaps boundary_swaps elem_integer_names_to_swap elem_integers_swaps", "ID Swap");

  return params;
}

RevolveGenerator::RevolveGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input(getMesh("input")),
    _axis_point(getParam<Point>("axis_point")),
    _axis_direction(getParam<Point>("axis_direction")),
    _revolving_angles(isParamValid("revolving_angles")
                          ? getParam<std::vector<Real>>("revolving_angles")
                          : std::vector<Real>(1, 360.0)),
    _subdomain_swaps(getParam<std::vector<std::vector<subdomain_id_type>>>("subdomain_swaps")),
    _boundary_swaps(getParam<std::vector<std::vector<boundary_id_type>>>("boundary_swaps")),
    _elem_integer_names_to_swap(getParam<std::vector<std::string>>("elem_integer_names_to_swap")),
    _elem_integers_swaps(
        getParam<std::vector<std::vector<std::vector<dof_id_type>>>>("elem_integers_swaps")),
    _clockwise(getParam<bool>("clockwise")),
    _nums_azimuthal_intervals(getParam<std::vector<unsigned int>>("nums_azimuthal_intervals")),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _has_start_boundary(isParamValid("start_boundary")),
    _start_boundary(isParamValid("start_boundary") ? getParam<boundary_id_type>("start_boundary")
                                                   : 0),
    _has_end_boundary(isParamValid("end_boundary")),
    _end_boundary(isParamValid("end_boundary") ? getParam<boundary_id_type>("end_boundary") : 0),
    _radius_correction_factor(1.0)
{
  if (_revolving_angles.size() != _nums_azimuthal_intervals.size())
    paramError("nums_azimuthal_intervals",
               "The number of azimuthal intervals should be the same as the number of revolving "
               "angles.");
  if (_subdomain_swaps.size() && (_subdomain_swaps.size() != _nums_azimuthal_intervals.size()))
    paramError(
        "subdomain_swaps",
        "If specified, 'subdomain_swaps' must be the same length as 'nums_azimuthal_intervals'.");

  if (_boundary_swaps.size() && (_boundary_swaps.size() != _nums_azimuthal_intervals.size()))
    paramError(
        "boundary_swaps",
        "If specified, 'boundary_swaps' must be the same length as 'nums_azimuthal_intervals'.");

  for (const auto & unit_elem_integers_swaps : _elem_integers_swaps)
    if (unit_elem_integers_swaps.size() != _nums_azimuthal_intervals.size())
      paramError("elem_integers_swaps",
                 "If specified, each element of 'elem_integers_swaps' must have the same length as "
                 "the length of 'nums_azimuthal_intervals'.");

  if (_elem_integers_swaps.size() &&
      _elem_integers_swaps.size() != _elem_integer_names_to_swap.size())
    paramError("elem_integers_swaps",
               "If specified, 'elem_integers_swaps' must have the same length as the length of "
               "'elem_integer_names_to_swap'.");

  _full_circle_revolving =
      MooseUtils::absoluteFuzzyEqual(
          std::accumulate(_revolving_angles.begin(), _revolving_angles.end(), 0), 360.0)
          ? true
          : false;
  if (MooseUtils::absoluteFuzzyGreaterThan(
          std::accumulate(_revolving_angles.begin(), _revolving_angles.end(), 0), 360.0))
    paramError("revolving_angles",
               "The sum of revolving angles should be less than or equal to 360.");

  if ((_has_start_boundary && _full_circle_revolving) ||
      (_has_end_boundary && _full_circle_revolving))
    paramError("full_circle_revolving",
               "starting or ending boundaries can only be assigned for partial revolving.");

  try
  {
    MooseMeshUtils::idSwapParametersProcessor(
        name(), "subdomain_swaps", _subdomain_swaps, _subdomain_swap_pairs);
  }
  catch (const MooseException & e)
  {
    paramError("subdomain_swaps", e.what());
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

  try
  {
    MooseMeshUtils::extraElemIntegerSwapParametersProcessor(name(),
                                                            _nums_azimuthal_intervals.size(),
                                                            _elem_integer_names_to_swap.size(),
                                                            _elem_integers_swaps,
                                                            _elem_integers_swap_pairs);
  }
  catch (const MooseException & e)
  {
    paramError("elem_integers_swaps", e.what());
  }
}

std::unique_ptr<MeshBase>
RevolveGenerator::generate()
{
  // Note: Inspired by AdvancedExtruderGenerator::generate()

  auto mesh = buildMeshBaseObject();

  // Only works for 1D and 2D input meshes
  if (_input->mesh_dimension() > 2)
    paramError("input", "This mesh generator only works for 1D and 2D input meshes.");

  mesh->set_mesh_dimension(_input->mesh_dimension() + 1);

  // Check if the element integer names are existent in the input mesh.
  for (const auto i : index_range(_elem_integer_names_to_swap))
    if (_input->has_elem_integer(_elem_integer_names_to_swap[i]))
      _elem_integer_indices_to_swap.push_back(
          _input->get_elem_integer_index(_elem_integer_names_to_swap[i]));
    else
      paramError("elem_integer_names_to_swap",
                 "Element ",
                 i + 1,
                 " of 'elem_integer_names_to_swap' is not a valid extra element integer of the "
                 "input mesh.");

  // prepare for transferring extra element integers from original mesh to the revolved mesh.
  const unsigned int num_extra_elem_integers = _input->n_elem_integers();
  std::vector<std::string> id_names;

  for (const auto i : make_range(num_extra_elem_integers))
  {
    id_names.push_back(_input->get_elem_integer_name(i));
    if (!mesh->has_elem_integer(id_names[i]))
      mesh->add_elem_integer(id_names[i]);
  }

  // retrieve subdomain/sideset/nodeset name maps
  const auto & input_subdomain_map = _input->get_subdomain_name_map();
  const auto & input_sideset_map = _input->get_boundary_info().get_sideset_name_map();
  const auto & input_nodeset_map = _input->get_boundary_info().get_nodeset_name_map();

  std::unique_ptr<MeshBase> input = std::move(_input);

  // If we're using a distributed mesh... then make sure we don't have any remote elements hanging
  // around
  if (!input->is_serial())
    mesh->delete_remote_elements();

  // check that subdomain swap sources exist in the mesh
  std::set<subdomain_id_type> blocks;
  input->subdomain_ids(blocks, true);
  for (const auto & swap_map : _subdomain_swap_pairs)
    for (const auto & [bid, tbid] : swap_map)
    {
      libmesh_ignore(tbid);
      if (blocks.count(bid) == 0)
        paramError("subdomain_swaps",
                   "Source subdomain " + std::to_string(bid) + " was not found in the mesh");
    }

  // Subdomain IDs for on-axis elements must be new
  std::set<subdomain_id_type> subdomain_ids_set;
  input->subdomain_ids(subdomain_ids_set);
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type tri_to_pyramid_subdomain_id_shift =
      std::max((int)max_subdomain_id, 1) + 1;
  const subdomain_id_type tri_to_tet_subdomain_id_shift =
      std::max((int)max_subdomain_id, 1) * 2 + 1;
  const subdomain_id_type quad_to_prism_subdomain_id_shift = std::max((int)max_subdomain_id, 1) + 1;
  const subdomain_id_type quad_to_pyramid_subdomain_id_shift =
      std::max((int)max_subdomain_id, 1) * 2 + 1;
  const subdomain_id_type quad_to_hi_pyramid_subdomain_id_shift =
      std::max((int)max_subdomain_id, 1) * 3 + 1;
  const subdomain_id_type edge_to_tri_subdomain_id_shift = std::max((int)max_subdomain_id, 1) + 1;

  // Get the centroid of the input mesh
  const auto input_centroid = MooseMeshUtils::meshCentroidCalculator(*input);
  const Point axis_centroid_cross = (input_centroid - _axis_point).cross(_axis_direction);

  if (MooseUtils::absoluteFuzzyEqual(axis_centroid_cross.norm(), 0.0))
    mooseError("The input mesh is either across the axis or overlapped with the axis!");

  Real inner_product_1d(0.0);
  bool inner_product_1d_initialized(false);
  // record ids of nodes on the axis
  std::vector<dof_id_type> node_ids_on_axis;
  for (const auto & node : input->node_ptr_range())
  {
    const Point axis_node_cross = (*node - _axis_point).cross(_axis_direction);
    // if the cross product is zero, then the node is on the axis
    if (!MooseUtils::absoluteFuzzyEqual(axis_node_cross.norm(), 0.0))
    {
      if (MooseUtils::absoluteFuzzyLessThan(axis_node_cross * axis_centroid_cross, 0.0))
        mooseError("The input mesh is across the axis.");
      else if (MooseUtils::absoluteFuzzyLessThan(axis_node_cross * axis_centroid_cross,
                                                 axis_centroid_cross.norm() *
                                                     axis_node_cross.norm()))
        mooseError("The input mesh is not in the same plane with the rotation axis.");
    }
    else
      node_ids_on_axis.push_back(node->id());

    // Only for 1D input mesh, we need to check if the axis is perpendicular to the input mesh
    if (input->mesh_dimension() == 1)
    {
      const Real temp_inner_product = (*node - _axis_point) * _axis_direction.unit();
      if (inner_product_1d_initialized)
      {
        if (!MooseUtils::absoluteFuzzyEqual(temp_inner_product, inner_product_1d))
          mooseError("The 1D input mesh is not perpendicular to the rotation axis.");
      }
      else
      {
        inner_product_1d_initialized = true;
        inner_product_1d = temp_inner_product;
      }
    }
  }

  // If there are any on-axis nodes, we need to check if there are any QUAD8 elements with one
  // vertex on the axis. If so, we need to replace it with a QUAD9 element.
  if (!node_ids_on_axis.empty())
  {
    // Sort the vector for using set_intersection
    std::sort(node_ids_on_axis.begin(), node_ids_on_axis.end());
    // For QUAD8 elements with one vertex on the axis, we need to replace it with a QUAD9 element
    std::set<subdomain_id_type> converted_quad8_subdomain_ids;
    for (const auto & elem : input->element_ptr_range())
    {
      if (elem->type() == QUAD8)
      {
        std::vector<dof_id_type> elem_vertex_node_ids;
        for (unsigned int i = 0; i < 4; i++)
        {
          elem_vertex_node_ids.push_back(elem->node_id(i));
        }
        std::sort(elem_vertex_node_ids.begin(), elem_vertex_node_ids.end());
        std::vector<dof_id_type> common_node_ids;
        std::set_intersection(node_ids_on_axis.begin(),
                              node_ids_on_axis.end(),
                              elem_vertex_node_ids.begin(),
                              elem_vertex_node_ids.end(),
                              std::back_inserter(common_node_ids));
        // Temporarily shift the subdomain ID to mark the element
        if (common_node_ids.size() == 1)
        {
          // we borrow quad_to_hi_pyramid_subdomain_id_shift here
          elem->subdomain_id() += quad_to_hi_pyramid_subdomain_id_shift;
          converted_quad8_subdomain_ids.emplace(elem->subdomain_id());
        }
      }
    }
    // Convert the recorded subdomains
    input->all_second_order_range(
        input->active_subdomain_set_elements_ptr_range(converted_quad8_subdomain_ids));
    // Restore the subdomain ID; we do not worry about repeated subdomain IDs because those QUAD9
    // will become PYRAMID and PRISM elements with new shifts
    for (auto elem : input->active_subdomain_set_elements_ptr_range(converted_quad8_subdomain_ids))
      elem->subdomain_id() -= quad_to_hi_pyramid_subdomain_id_shift;
  }

  // We should only record this info after QUAD8->QUAD9 conversion
  dof_id_type orig_elem = input->n_elem();
  dof_id_type orig_nodes = input->n_nodes();

#ifdef LIBMESH_ENABLE_UNIQUE_ID
  // Add the number of original elements as revolving may create two elements per layer for one
  // original element
  unique_id_type orig_unique_ids = input->parallel_max_unique_id() + orig_elem;
#endif

  // get rotation vectors
  const auto rotation_vectors = rotationVectors(_axis_point, _axis_direction, input_centroid);

  unsigned int order = 1;

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  const BoundaryInfo & input_boundary_info = input->get_boundary_info();

  const unsigned int total_num_azimuthal_intervals =
      std::accumulate(_nums_azimuthal_intervals.begin(), _nums_azimuthal_intervals.end(), 0);
  // We know a priori how many elements we'll need
  // In the worst case, all quad elements will become two elements per layer
  mesh->reserve_elem(total_num_azimuthal_intervals * orig_elem * 2);
  const dof_id_type elem_id_shift = total_num_azimuthal_intervals * orig_elem;

  // Look for higher order elements which introduce an extra layer
  std::set<ElemType> higher_orders = {EDGE3, TRI6, TRI7, QUAD8, QUAD9};
  std::vector<ElemType> types;
  MeshTools::elem_types(*input, types);
  for (const auto elem_type : types)
    if (higher_orders.count(elem_type))
      order = 2;
  mesh->comm().max(order);

  // Collect azimuthal angles and use them to calculate the correction factor if applicable
  std::vector<Real> azi_array;
  for (const auto & i : index_range(_revolving_angles))
  {
    const Real section_start_angle =
        azi_array.empty() ? 0.0 : (azi_array.back() + _unit_angles.back());
    _unit_angles.push_back(_revolving_angles[i] / _nums_azimuthal_intervals[i] / order);
    for (unsigned int j = 0; j < _nums_azimuthal_intervals[i] * order; j++)
      azi_array.push_back(section_start_angle + _unit_angles.back() * (Real)j);
  }
  if (_preserve_volumes)
  {
    _radius_correction_factor = PolygonalMeshGenerationUtils::radiusCorrectionFactor(
        azi_array, _full_circle_revolving, order);

    // In the meanwhile, modify the input mesh for radius correction if applicable
    for (const auto & node : input->node_ptr_range())
      nodeModification(*node);
  }

  mesh->reserve_nodes(
      (order * total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
      orig_nodes);

  // Container to catch the boundary IDs handed back by the BoundaryInfo object
  std::vector<boundary_id_type> ids_to_copy;

  Point old_distance;
  Point current_distance;
  if (!_clockwise)
    std::transform(_unit_angles.begin(),
                   _unit_angles.end(),
                   _unit_angles.begin(),
                   [](auto & c) { return c * (-1.0) * M_PI / 180.0; });
  else
    std::transform(_unit_angles.begin(),
                   _unit_angles.end(),
                   _unit_angles.begin(),
                   [](auto & c) { return c * M_PI / 180.0; });
  std::vector<dof_id_type> nodes_on_axis;

  for (const auto & node : input->node_ptr_range())
  {
    // Calculate the radius and corresponding center point on the rotation axis
    // If the radius is 0, then the node is on the axis
    const auto radius_and_center = getRotationCenterAndRadius(*node, _axis_point, _axis_direction);
    const bool isOnAxis = MooseUtils::absoluteFuzzyEqual(radius_and_center.first, 0.0);
    if (isOnAxis)
    {
      nodes_on_axis.push_back(node->id());
    }

    unsigned int current_node_layer = 0;

    old_distance.zero();

    const unsigned int num_rotations = _revolving_angles.size();
    for (unsigned int e = 0; e < num_rotations; e++)
    {
      auto num_layers = _nums_azimuthal_intervals[e];

      auto angle = _unit_angles[e];

      const auto base_angle =
          std::accumulate(_revolving_angles.begin(), _revolving_angles.begin() + e, 0.0) / 180.0 *
          M_PI;

      for (unsigned int k = 0;
           k < order * num_layers + (e == 0 ? 1 : 0) -
                   (e == num_rotations - 1 ? (unsigned int)_full_circle_revolving : 0);
           ++k)
      {
        bool is_node_created(false);
        if (!isOnAxis)
        {
          // For the first layer we don't need to move
          if (e == 0 && k == 0)
            current_distance.zero();
          else
          {
            auto layer_index = (k - (e == 0 ? 1 : 0)) + 1;

            // Calculate the rotation angle in XY Plane
            const Point vector_xy =
                Point(-2.0 * radius_and_center.first *
                          std::sin((base_angle + angle * (Real)layer_index) / 2.0) *
                          std::sin((base_angle + angle * (Real)layer_index) / 2.0),
                      2.0 * radius_and_center.first *
                          std::sin((base_angle + angle * (Real)layer_index) / 2.0) *
                          std::cos((base_angle + angle * (Real)layer_index) / 2.0),
                      0.0);
            current_distance = Point(rotation_vectors[0] * vector_xy,
                                     rotation_vectors[1] * vector_xy,
                                     rotation_vectors[2] * vector_xy);
          }

          is_node_created = true;
        }
        else if (e == 0 && k == 0)
        {
          // On-axis nodes are only added once
          current_distance.zero();
          is_node_created = true;
        }

        if (is_node_created)
        {
          Node * new_node = mesh->add_point(*node + current_distance,
                                            node->id() + (current_node_layer * orig_nodes),
                                            node->processor_id());
#ifdef LIBMESH_ENABLE_UNIQUE_ID
          // Let's give the base nodes of the revolved mesh the same
          // unique_ids as the source mesh, in case anyone finds that
          // a useful map to preserve.
          const unique_id_type uid =
              (current_node_layer == 0)
                  ? node->unique_id()
                  : (orig_unique_ids + (current_node_layer - 1) * (orig_nodes + orig_elem * 2) +
                     node->id());
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
        }

        current_node_layer++;
      }
    }
  }

  for (const auto & elem : input->element_ptr_range())
  {
    const ElemType etype = elem->type();

    // revolving currently only works on coarse meshes
    mooseAssert(!elem->parent(), "RevolveGenerator only works on coarse meshes.");

    unsigned int current_layer = 0;

    const unsigned int num_rotations = _revolving_angles.size();

    for (unsigned int e = 0; e < num_rotations; e++)
    {
      auto num_layers = _nums_azimuthal_intervals[e];

      for (unsigned int k = 0; k < num_layers; ++k)
      {
        std::unique_ptr<Elem> new_elem;
        std::unique_ptr<Elem> new_elem_1;
        bool is_flipped(false);
        // In some cases, two elements per layer are generated by revolving one element. So we
        // reserve an additional flag for the potential second element.
        bool is_flipped_additional(false);
        dof_id_type axis_node_case(-1);
        std::vector<std::pair<dof_id_type, dof_id_type>> side_pairs;
        switch (etype)
        {
          case EDGE2:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a quad4 element is created
            // 2. One of the nodes is on the axis
            //    Then a tri3 element is created
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createQUADfromEDGE(QUAD4,
                                 elem,
                                 mesh,
                                 new_elem,
                                 current_layer,
                                 orig_nodes,
                                 total_num_azimuthal_intervals,
                                 side_pairs,
                                 is_flipped);
            }
            else
            {
              createTRIfromEDGE(nodes_cates,
                                TRI3,
                                elem,
                                mesh,
                                new_elem,
                                current_layer,
                                orig_nodes,
                                total_num_azimuthal_intervals,
                                side_pairs,
                                axis_node_case,
                                is_flipped);
            }
            break;
          }
          case EDGE3:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a QUAD9 element is created
            // 2. One of the nodes is on the axis
            //    Then a TRI7 element is created
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createQUADfromEDGE(QUAD9,
                                 elem,
                                 mesh,
                                 new_elem,
                                 current_layer,
                                 orig_nodes,
                                 total_num_azimuthal_intervals,
                                 side_pairs,
                                 is_flipped);
            }
            else
            {
              createTRIfromEDGE(nodes_cates,
                                TRI7,
                                elem,
                                mesh,
                                new_elem,
                                current_layer,
                                orig_nodes,
                                total_num_azimuthal_intervals,
                                side_pairs,
                                axis_node_case,
                                is_flipped);
            }
            break;
          }
          case TRI3:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a prism6 element is created
            // 2. One of the nodes is on the axis
            //    Then a pyramid5 element is created
            // 3. Two of the nodes are on the axis
            //    Then a tet4 element is created
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createPRISMfromTRI(PRISM6,
                                 elem,
                                 mesh,
                                 new_elem,
                                 current_layer,
                                 orig_nodes,
                                 total_num_azimuthal_intervals,
                                 side_pairs,
                                 is_flipped);
            }
            else if (nodes_cates.first.size() == 1)
            {
              createPYRAMIDfromTRI(nodes_cates,
                                   PYRAMID5,
                                   elem,
                                   mesh,
                                   new_elem,
                                   current_layer,
                                   orig_nodes,
                                   total_num_azimuthal_intervals,
                                   side_pairs,
                                   axis_node_case,
                                   is_flipped);
            }
            else if (nodes_cates.first.size() == 2)
            {
              createTETfromTRI(nodes_cates,
                               TET4,
                               elem,
                               mesh,
                               new_elem,
                               current_layer,
                               orig_nodes,
                               total_num_azimuthal_intervals,
                               side_pairs,
                               axis_node_case,
                               is_flipped);
            }
            else
              mooseError("A degenerate TRI3 elements overlapped with the rotation axis cannot be "
                         "revolved.");

            break;
          }
          case TRI6:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a prism18 element is created
            // 2. One of the nodes is on the axis
            //    Then a pyramid13 element is created
            // 3. Three of the nodes are on the axis
            //    Then a tet10 element is created
            // NOTE: We do not support two nodes on the axis for tri6 elements
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createPRISMfromTRI(PRISM18,
                                 elem,
                                 mesh,
                                 new_elem,
                                 current_layer,
                                 orig_nodes,
                                 total_num_azimuthal_intervals,
                                 side_pairs,
                                 is_flipped);
            }
            else if (nodes_cates.first.size() == 1)
            {
              createPYRAMIDfromTRI(nodes_cates,
                                   PYRAMID13,
                                   elem,
                                   mesh,
                                   new_elem,
                                   current_layer,
                                   orig_nodes,
                                   total_num_azimuthal_intervals,
                                   side_pairs,
                                   axis_node_case,
                                   is_flipped);
            }
            else if (nodes_cates.first.size() == 3)
            {
              createTETfromTRI(nodes_cates,
                               TET10,
                               elem,
                               mesh,
                               new_elem,
                               current_layer,
                               orig_nodes,
                               total_num_azimuthal_intervals,
                               side_pairs,
                               axis_node_case,
                               is_flipped);
            }
            else
              mooseError(
                  "You either have a degenerate TRI6 element, or the mid-point of the "
                  "on-axis edge is not colinear with the two vertices, which is not supported.");
            break;
          }
          case TRI7:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a prism21 element is created
            // 2. One of the nodes is on the axis
            //    Then a pyramid18 element is created
            // 3. Three of the nodes are on the axis
            //    Then a tet14 element is created
            // NOTE: We do not support two nodes on the axis for tri7 elements
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createPRISMfromTRI(PRISM21,
                                 elem,
                                 mesh,
                                 new_elem,
                                 current_layer,
                                 orig_nodes,
                                 total_num_azimuthal_intervals,
                                 side_pairs,
                                 is_flipped);
            }
            else if (nodes_cates.first.size() == 1)
            {
              createPYRAMIDfromTRI(nodes_cates,
                                   PYRAMID18,
                                   elem,
                                   mesh,
                                   new_elem,
                                   current_layer,
                                   orig_nodes,
                                   total_num_azimuthal_intervals,
                                   side_pairs,
                                   axis_node_case,
                                   is_flipped);
            }
            else if (nodes_cates.first.size() == 3)
            {
              createTETfromTRI(nodes_cates,
                               TET14,
                               elem,
                               mesh,
                               new_elem,
                               current_layer,
                               orig_nodes,
                               total_num_azimuthal_intervals,
                               side_pairs,
                               axis_node_case,
                               is_flipped);
            }
            else
              mooseError("You either have a degenerate TRI6 element, or the mid-point of the "
                         "on-axis edge of the TRI6 element is not colinear with the two vertices, "
                         "which is not supported.");
            break;
          }
          case QUAD4:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a hex8 element is created
            // 2. One of the nodes is on the axis
            //    Then a pyramid5 element and a prism6 element are created
            // 3. Two of the nodes are on the axis
            //    Then a prism6 is created
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createHEXfromQUAD(HEX8,
                                elem,
                                mesh,
                                new_elem,
                                current_layer,
                                orig_nodes,
                                total_num_azimuthal_intervals,
                                side_pairs,
                                is_flipped);
            }
            else if (nodes_cates.first.size() == 1)
            {
              createPYRAMIDPRISMfromQUAD(nodes_cates,
                                         PYRAMID5,
                                         PRISM6,
                                         elem,
                                         mesh,
                                         new_elem,
                                         new_elem_1,
                                         current_layer,
                                         orig_nodes,
                                         total_num_azimuthal_intervals,
                                         side_pairs,
                                         axis_node_case,
                                         is_flipped,
                                         is_flipped_additional);
            }
            else if (nodes_cates.first.size() == 2)
            {
              createPRISMfromQUAD(nodes_cates,
                                  PRISM6,
                                  elem,
                                  mesh,
                                  new_elem,
                                  current_layer,
                                  orig_nodes,
                                  total_num_azimuthal_intervals,
                                  side_pairs,
                                  axis_node_case,
                                  is_flipped);
            }

            else
              mooseError("Degenerate QUAD4 element with 3 or more aligned nodes cannot be "
                         "azimuthally revolved");

            break;
          }
          case QUAD8:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a hex20 element is created
            // 2. One of the nodes is on the axis
            //    In that case, it is already converted to a QUAD9 element before,
            //    SO we do not need to worry about this case
            // 3. Three of the nodes are on the axis
            //    Then a prism15 is created
            // NOTE: We do not support two nodes on the axis for quad8 elements
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createHEXfromQUAD(HEX20,
                                elem,
                                mesh,
                                new_elem,
                                current_layer,
                                orig_nodes,
                                total_num_azimuthal_intervals,
                                side_pairs,
                                is_flipped);
            }
            else if (nodes_cates.first.size() == 3)
            {
              createPRISMfromQUAD(nodes_cates,
                                  PRISM15,
                                  elem,
                                  mesh,
                                  new_elem,
                                  current_layer,
                                  orig_nodes,
                                  total_num_azimuthal_intervals,
                                  side_pairs,
                                  axis_node_case,
                                  is_flipped);
            }
            else
              mooseError("You either have a degenerate QUAD8 element, or the mid-point of the "
                         "on-axis edge of the QUAD8 element is not colinear with the two vertices, "
                         "which is not supported.");

            break;
          }
          case QUAD9:
          {
            // Possible scenarios:
            // 1. None of the nodes are on the axis
            //    Then a hex27 element is created
            // 2. One of the nodes is on the axis
            //    Then a pyramid14 element and a prism18 element are created
            // 3. Two of the nodes are on the axis
            //    Then a prism18 is created
            // (we do not create prism20/21 here just to make prism18 the only possible prism
            // elements for simplicity)
            const auto nodes_cates = onAxisNodesIdentifier(*elem, nodes_on_axis);
            if (nodes_cates.first.empty())
            {
              createHEXfromQUAD(HEX27,
                                elem,
                                mesh,
                                new_elem,
                                current_layer,
                                orig_nodes,
                                total_num_azimuthal_intervals,
                                side_pairs,
                                is_flipped);
            }
            else if (nodes_cates.first.size() == 1)
            {
              createPYRAMIDPRISMfromQUAD(nodes_cates,
                                         PYRAMID14,
                                         PRISM18,
                                         elem,
                                         mesh,
                                         new_elem,
                                         new_elem_1,
                                         current_layer,
                                         orig_nodes,
                                         total_num_azimuthal_intervals,
                                         side_pairs,
                                         axis_node_case,
                                         is_flipped,
                                         is_flipped_additional);
            }
            else if (nodes_cates.first.size() == 3)
            {
              createPRISMfromQUAD(nodes_cates,
                                  PRISM18,
                                  elem,
                                  mesh,
                                  new_elem,
                                  current_layer,
                                  orig_nodes,
                                  total_num_azimuthal_intervals,
                                  side_pairs,
                                  axis_node_case,
                                  is_flipped);
            }
            else
              mooseError("You either have a degenerate QUAD9 element, or the mid-point of the "
                         "on-axis edge of the QUAD9 element is not colinear with the two vertices, "
                         "which is not supported.");
            break;
          }
          default:
            mooseError("The input mesh contains unsupported element type(s).");
        }
        new_elem->set_id(elem->id() + (current_layer * orig_elem));
        new_elem->processor_id() = elem->processor_id();
        if (new_elem_1)
        {
          new_elem_1->set_id(elem->id() + (current_layer * orig_elem) + elem_id_shift);
          new_elem_1->processor_id() = elem->processor_id();
        }

#ifdef LIBMESH_ENABLE_UNIQUE_ID
        // Let's give the base elements of the revolved mesh the same
        // unique_ids as the source mesh, in case anyone finds that
        // a useful map to preserve.
        const unique_id_type uid =
            (current_layer == 0)
                ? elem->unique_id()
                : (orig_unique_ids + (current_layer - 1) * (orig_nodes + orig_elem * 2) +
                   orig_nodes + elem->id());

        new_elem->set_unique_id(uid);

        // Special case for extra elements
        if (new_elem_1)
        {
          const unique_id_type uid_1 =
              (current_layer == 0)
                  ? (elem->id() + orig_unique_ids - orig_elem)
                  : (orig_unique_ids + (current_layer - 1) * (orig_nodes + orig_elem * 2) +
                     orig_nodes + orig_elem + elem->id());

          new_elem_1->set_unique_id(uid_1);
        }
#endif

        // maintain the subdomain_id
        switch (etype)
        {
          case EDGE2:
            switch (new_elem->type())
            {
              case QUAD4:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case TRI3:
                new_elem->subdomain_id() = edge_to_tri_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false,
                            "impossible element type generated by revolving an EDGE2 element");
            }
            break;
          case EDGE3:
            switch (new_elem->type())
            {
              case QUAD9:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case TRI7:
                new_elem->subdomain_id() = edge_to_tri_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false,
                            "impossible element type generated by revolving an EDGE3 element");
            }
            break;
          case TRI3:
            switch (new_elem->type())
            {
              case PRISM6:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case PYRAMID5:
                new_elem->subdomain_id() = tri_to_pyramid_subdomain_id_shift + elem->subdomain_id();
                break;
              case TET4:
                new_elem->subdomain_id() = tri_to_tet_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false, "impossible element type generated by revolving a TRI3 element");
            }
            break;
          case TRI6:
            switch (new_elem->type())
            {
              case PRISM18:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case PYRAMID13:
                new_elem->subdomain_id() = tri_to_pyramid_subdomain_id_shift + elem->subdomain_id();
                break;
              case TET10:
                new_elem->subdomain_id() = tri_to_tet_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false, "impossible element type generated by revolving a TRI6 element");
            }
            break;
          case TRI7:
            switch (new_elem->type())
            {
              case PRISM21:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case PYRAMID18:
                new_elem->subdomain_id() = tri_to_pyramid_subdomain_id_shift + elem->subdomain_id();
                break;
              case TET14:
                new_elem->subdomain_id() = tri_to_tet_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false, "impossible element type generated by revolving a TRI7 element");
            }
            break;
          case QUAD4:
            switch (new_elem->type())
            {
              case HEX8:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case PRISM6:
                new_elem->subdomain_id() = quad_to_prism_subdomain_id_shift + elem->subdomain_id();
                break;
              case PYRAMID5:
                new_elem->subdomain_id() =
                    quad_to_pyramid_subdomain_id_shift + elem->subdomain_id();
                new_elem_1->subdomain_id() =
                    quad_to_prism_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false,
                            "impossible element type generated by revolving a QUAD4 element");
            }
            break;
          case QUAD8:
            switch (new_elem->type())
            {
              case HEX20:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case PRISM15:
                new_elem->subdomain_id() = quad_to_prism_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false,
                            "impossible element type generated by revolving a QUAD8 element");
            }
            break;
          case QUAD9:
            switch (new_elem->type())
            {
              case HEX27:
                new_elem->subdomain_id() = elem->subdomain_id();
                break;
              case PRISM18:
                new_elem->subdomain_id() =
                    quad_to_hi_pyramid_subdomain_id_shift + elem->subdomain_id();
                break;
              case PYRAMID14:
                new_elem->subdomain_id() =
                    quad_to_pyramid_subdomain_id_shift + elem->subdomain_id();
                new_elem_1->subdomain_id() =
                    quad_to_hi_pyramid_subdomain_id_shift + elem->subdomain_id();
                break;
              default:
                mooseAssert(false,
                            "impossible element type generated by revolving a QUAD9 element");
            }
            break;
          default:
            mooseAssert(false,
                        "The input mesh contains unsupported element type(s), which should have "
                        "been checked in prior steps in this code.");
        }

        if (_subdomain_swap_pairs.size())
        {
          auto & revolving_swap_pairs = _subdomain_swap_pairs[e];

          auto new_id_it = revolving_swap_pairs.find(elem->subdomain_id());

          if (new_id_it != revolving_swap_pairs.end())
          {
            new_elem->subdomain_id() =
                new_elem->subdomain_id() - elem->subdomain_id() + new_id_it->second;
            if (new_elem_1)
              new_elem_1->subdomain_id() =
                  new_elem_1->subdomain_id() - elem->subdomain_id() + new_id_it->second;
          }
        }

        Elem * added_elem = mesh->add_elem(std::move(new_elem));
        Elem * added_elem_1 = NULL;

        if (new_elem_1)
          added_elem_1 = mesh->add_elem(std::move(new_elem_1));

        // maintain extra integers
        for (unsigned int i = 0; i < num_extra_elem_integers; i++)
        {
          added_elem->set_extra_integer(i, elem->get_extra_integer(i));
          if (added_elem_1)
            added_elem_1->set_extra_integer(i, elem->get_extra_integer(i));
        }

        if (_elem_integers_swap_pairs.size())
        {
          for (unsigned int i = 0; i < _elem_integer_indices_to_swap.size(); i++)
          {
            auto & elevation_extra_swap_pairs =
                _elem_integers_swap_pairs[i * _nums_azimuthal_intervals.size() + e];

            auto new_extra_id_it = elevation_extra_swap_pairs.find(
                elem->get_extra_integer(_elem_integer_indices_to_swap[i]));

            if (new_extra_id_it != elevation_extra_swap_pairs.end())
            {
              added_elem->set_extra_integer(_elem_integer_indices_to_swap[i],
                                            new_extra_id_it->second);
              if (added_elem_1)
                added_elem_1->set_extra_integer(_elem_integer_indices_to_swap[i],
                                                new_extra_id_it->second);
            }
          }
        }

        // Copy any old boundary ids on all sides
        for (auto s : elem->side_index_range())
        {
          input_boundary_info.boundary_ids(elem, s, ids_to_copy);
          std::vector<boundary_id_type> ids_to_copy_swapped;
          if (_boundary_swap_pairs.empty())
            ids_to_copy_swapped = ids_to_copy;
          else
            for (const auto & id_to_copy : ids_to_copy)
              ids_to_copy_swapped.push_back(_boundary_swap_pairs[e].count(id_to_copy)
                                                ? _boundary_swap_pairs[e][id_to_copy]
                                                : id_to_copy);

          switch (etype)
          {
            case EDGE2:
              switch (added_elem->type())
              {
                case QUAD4:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s == 0 ? 3 : 1), ids_to_copy_swapped);
                  break;
                case TRI3:
                  if (s != axis_node_case)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(s), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving an EDGE2 element");
              }
              break;
            case EDGE3:
              switch (added_elem->type())
              {
                case QUAD9:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s == 0 ? 3 : 1), ids_to_copy_swapped);
                  break;
                case TRI7:
                  if (s != axis_node_case)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(s), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving an EDGE3 element");
              }
              break;
            case TRI3:
              switch (added_elem->type())
              {
                case PRISM6:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s + 1), ids_to_copy_swapped);
                  break;
                case PYRAMID5:
                  if ((s + 3 - axis_node_case) % 3 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  else if ((s + 3 - axis_node_case) % 3 == 1)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(4), ids_to_copy_swapped);
                  else
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(1), ids_to_copy_swapped);
                  break;
                case TET4:
                  if ((s + 3 - axis_node_case) % 3 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  else if ((s + 3 - axis_node_case) % 3 == 2)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving a TRI3 element");
              }
              break;
            case TRI6:
              switch (added_elem->type())
              {
                case PRISM18:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s + 1), ids_to_copy_swapped);
                  break;
                case PYRAMID13:
                  if ((s + 3 - axis_node_case) % 3 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  else if ((s + 3 - axis_node_case) % 3 == 1)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(4), ids_to_copy_swapped);
                  else
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(1), ids_to_copy_swapped);
                  break;
                case TET10:
                  if ((s + 3 - axis_node_case) % 3 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  else if ((s + 3 - axis_node_case) % 3 == 2)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving a TRI6 element");
              }
              break;
            case TRI7:
              switch (added_elem->type())
              {
                case PRISM21:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s + 1), ids_to_copy_swapped);
                  break;
                case PYRAMID18:
                  if ((s + 3 - axis_node_case) % 3 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  else if ((s + 3 - axis_node_case) % 3 == 1)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(4), ids_to_copy_swapped);
                  else
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(1), ids_to_copy_swapped);
                  break;
                case TET14:
                  if ((s + 3 - axis_node_case) % 3 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  else if ((s + 3 - axis_node_case) % 3 == 2)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving a TRI7 element");
              }
              break;
            case QUAD4:
              switch (added_elem->type())
              {
                case HEX8:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s + 1), ids_to_copy_swapped);
                  break;
                case PRISM6:
                  if ((s + 4 - axis_node_case) % 4 == 1)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(4), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 2)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 3)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(0), ids_to_copy_swapped);
                  break;
                case PYRAMID5:
                  if ((s + 4 - axis_node_case) % 4 == 3)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(1), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 1)
                    boundary_info.add_side(
                        added_elem_1, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  else
                    boundary_info.add_side(
                        added_elem_1, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving a QUAD4 element");
              }
              break;
            case QUAD8:
              switch (added_elem->type())
              {
                case HEX20:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s + 1), ids_to_copy_swapped);
                  break;
                case PRISM15:
                  if ((s + 4 - axis_node_case) % 4 == 1)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(4), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 2)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 3)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(0), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving a QUAD8 element");
              }
              break;
            case QUAD9:
              switch (added_elem->type())
              {
                case HEX27:
                  boundary_info.add_side(
                      added_elem, cast_int<unsigned short>(s + 1), ids_to_copy_swapped);
                  break;
                case PRISM18:
                  if ((s + 4 - axis_node_case) % 4 == 1)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(4), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 2)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 3)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(0), ids_to_copy_swapped);
                  break;
                case PYRAMID14:
                  if ((s + 4 - axis_node_case) % 4 == 3)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(1), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 0)
                    boundary_info.add_side(
                        added_elem, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  else if ((s + 4 - axis_node_case) % 4 == 1)
                    boundary_info.add_side(
                        added_elem_1, cast_int<unsigned short>(3), ids_to_copy_swapped);
                  else
                    boundary_info.add_side(
                        added_elem_1, cast_int<unsigned short>(2), ids_to_copy_swapped);
                  break;
                default:
                  mooseAssert(false,
                              "impossible element type generated by revolving a QUAD9 element");
              }
              break;
            default:
              mooseAssert(false,
                          "The input mesh contains unsupported element type(s), which should have "
                          "been checked in prior steps in this code.");
          }
        }

        if (current_layer == 0 && _has_start_boundary)
        {
          boundary_info.add_side(
              added_elem, is_flipped ? side_pairs[0].second : side_pairs[0].first, _start_boundary);
          if (side_pairs.size() > 1)
            boundary_info.add_side(added_elem_1,
                                   is_flipped_additional ? side_pairs[1].second
                                                         : side_pairs[1].first,
                                   _start_boundary);
        }

        if (current_layer == num_layers - 1 && _has_end_boundary)
        {
          boundary_info.add_side(
              added_elem, is_flipped ? side_pairs[0].first : side_pairs[0].second, _end_boundary);
          if (side_pairs.size() > 1)
            boundary_info.add_side(added_elem_1,
                                   is_flipped_additional ? side_pairs[1].first
                                                         : side_pairs[1].second,
                                   _end_boundary);
        }
        current_layer++;
      }
    }
  }

#ifdef LIBMESH_ENABLE_UNIQUE_ID
  // Update the value of next_unique_id based on newly created nodes and elements
  // Note: the calculation here is quite conservative to ensure uniqueness
  unsigned int total_new_node_layers = total_num_azimuthal_intervals * order;
  unsigned int new_unique_ids = orig_unique_ids + (total_new_node_layers - 1) * orig_elem * 2 +
                                total_new_node_layers * orig_nodes;
  mesh->set_next_unique_id(new_unique_ids);
#endif

  // Copy all the subdomain/sideset/nodeset name maps to the revolved mesh
  if (!input_subdomain_map.empty())
    mesh->set_subdomain_name_map().insert(input_subdomain_map.begin(), input_subdomain_map.end());
  if (!input_sideset_map.empty())
    mesh->get_boundary_info().set_sideset_name_map().insert(input_sideset_map.begin(),
                                                            input_sideset_map.end());
  if (!input_nodeset_map.empty())
    mesh->get_boundary_info().set_nodeset_name_map().insert(input_nodeset_map.begin(),
                                                            input_nodeset_map.end());

  mesh->remove_orphaned_nodes();
  mesh->renumber_nodes_and_elements();
  mesh->set_isnt_prepared();

  return mesh;
}

std::pair<Real, Point>
RevolveGenerator::getRotationCenterAndRadius(const Point & p_ext,
                                             const Point & p_axis,
                                             const Point & dir_axis) const
{
  // First use point product to get the distance between the axis point and the projection of the
  // external point on the axis
  const Real dist = (p_ext - p_axis) * dir_axis.unit();
  const Point center_pt = p_axis + dist * dir_axis.unit();
  // Then get the radius
  const Real radius = (p_ext - center_pt).norm();
  return std::make_pair(radius, center_pt);
}

std::vector<Point>
RevolveGenerator::rotationVectors(const Point & p_axis,
                                  const Point & dir_axis,
                                  const Point & p_input) const
{
  // To make the rotation mathematically simple, we perform rotation in a coordination system
  // (x',y',z') defined by rotation axis and the mesh to be rotated.
  // z' is the rotation axis, which is trivial dir_axis.unit()
  const Point z_prime = dir_axis.unit();
  // the x' and z' should form the plane that accommodates input mesh
  const Point x_prime = ((p_input - p_axis) - ((p_input - p_axis) * z_prime) * z_prime).unit();
  const Point y_prime = z_prime.cross(x_prime);
  // Then we transform things back to the original coordination system (x,y,z), which is trivial
  // (1,0,0), (0,1,0), (0,0,1)
  return {{x_prime(0), y_prime(0), z_prime(0)},
          {x_prime(1), y_prime(1), z_prime(1)},
          {x_prime(2), y_prime(2), z_prime(2)}};
}

std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>>
RevolveGenerator::onAxisNodesIdentifier(const Elem & elem,
                                        const std::vector<dof_id_type> & nodes_on_axis) const
{
  std::vector<dof_id_type> nodes_on_axis_in_elem;
  std::vector<dof_id_type> nodes_not_on_axis_in_elem;
  for (unsigned int i = 0; i < elem.n_nodes(); i++)
  {
    const auto node_id = elem.node_id(i);
    if (std::find(nodes_on_axis.begin(), nodes_on_axis.end(), node_id) != nodes_on_axis.end())
    {
      nodes_on_axis_in_elem.push_back(i);
    }
    else
    {
      nodes_not_on_axis_in_elem.push_back(i);
    }
  }
  return std::make_pair(nodes_on_axis_in_elem, nodes_not_on_axis_in_elem);
}

void
RevolveGenerator::nodeModification(Node & node)
{
  const Point axis_component =
      ((node - _axis_point) * _axis_direction.unit()) * _axis_direction.unit();
  const Point rad_component = ((node - _axis_point) - axis_component) * _radius_correction_factor;
  node = _axis_point + axis_component + rad_component;
}

void
RevolveGenerator::createQUADfromEDGE(const ElemType quad_elem_type,
                                     const Elem * elem,
                                     const std::unique_ptr<MeshBase> & mesh,
                                     std::unique_ptr<Elem> & new_elem,
                                     const int current_layer,
                                     const unsigned int orig_nodes,
                                     const unsigned int total_num_azimuthal_intervals,
                                     std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
                                     bool & is_flipped) const
{
  if (quad_elem_type != QUAD4 && quad_elem_type != QUAD9)
    mooseError("Unsupported element type", quad_elem_type);

  side_pairs.push_back(std::make_pair(0, 2));
  const unsigned int order = quad_elem_type == QUAD4 ? 1 : 2;

  new_elem = std::make_unique<Quad4>();
  if (quad_elem_type == QUAD9)
  {
    new_elem = std::make_unique<Quad9>();
    new_elem->set_node(4) =
        mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(5) =
        mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(6) =
        mesh->node_ptr(elem->node_ptr(2)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(7) =
        mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(8) =
        mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer * 2 + 1) * orig_nodes));
  }

  new_elem->set_node(0) =
      mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(1) =
      mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(3) =
      mesh->node_ptr(elem->node_ptr(0)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(2) =
      mesh->node_ptr(elem->node_ptr(1)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 0, 3);
    MooseMeshUtils::swapNodesInElem(*new_elem, 1, 2);
    if (quad_elem_type == QUAD9)
      MooseMeshUtils::swapNodesInElem(*new_elem, 4, 6);
    is_flipped = true;
  }
}

void
RevolveGenerator::createTRIfromEDGE(
    const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
    const ElemType tri_elem_type,
    const Elem * elem,
    const std::unique_ptr<MeshBase> & mesh,
    std::unique_ptr<Elem> & new_elem,
    const int current_layer,
    const unsigned int orig_nodes,
    const unsigned int total_num_azimuthal_intervals,
    std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
    dof_id_type & axis_node_case,
    bool & is_flipped) const
{
  if (tri_elem_type != TRI3 && tri_elem_type != TRI7)
    mooseError("Unsupported element type", tri_elem_type);

  side_pairs.push_back(std::make_pair(0, 2));
  const unsigned int order = tri_elem_type == TRI3 ? 1 : 2;
  axis_node_case = nodes_cates.first.front();

  new_elem = std::make_unique<Tri3>();
  if (tri_elem_type == TRI7)
  {
    new_elem = std::make_unique<Tri7>();
    new_elem->set_node(3) =
        mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(4) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 2)->id() +
                                           ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(5) =
        mesh->node_ptr(elem->node_ptr(2)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(6) =
        mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer * 2 + 1) * orig_nodes));
  }

  new_elem->set_node(0) = mesh->node_ptr(elem->node_ptr(axis_node_case)->id());
  new_elem->set_node(1) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 2)->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(2) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 2)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 1, 2);
    if (tri_elem_type == TRI7)
      MooseMeshUtils::swapNodesInElem(*new_elem, 3, 5);
    is_flipped = true;
  }
}

void
RevolveGenerator::createPRISMfromTRI(const ElemType prism_elem_type,
                                     const Elem * elem,
                                     const std::unique_ptr<MeshBase> & mesh,
                                     std::unique_ptr<Elem> & new_elem,
                                     const int current_layer,
                                     const unsigned int orig_nodes,
                                     const unsigned int total_num_azimuthal_intervals,
                                     std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
                                     bool & is_flipped) const
{
  if (prism_elem_type != PRISM6 && prism_elem_type != PRISM18 && prism_elem_type != PRISM21)
    mooseError("unsupported situation");

  side_pairs.push_back(std::make_pair(0, 4));
  const unsigned int order = prism_elem_type == PRISM6 ? 1 : 2;

  new_elem = std::make_unique<Prism6>();

  if (order == 2)
  {
    new_elem = std::make_unique<Prism18>();
    if (prism_elem_type == PRISM21)
    {
      new_elem = std::make_unique<Prism21>();
      new_elem->set_node(18) =
          mesh->node_ptr(elem->node_ptr(6)->id() + (current_layer * 2 * orig_nodes));
      new_elem->set_node(19) = mesh->node_ptr(
          elem->node_ptr(6)->id() +
          ((current_layer + 1) %
           (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) * 2 *
           orig_nodes));
      new_elem->set_node(20) =
          mesh->node_ptr(elem->node_ptr(6)->id() + ((current_layer * 2 + 1) * orig_nodes));
    }
    new_elem->set_node(6) =
        mesh->node_ptr(elem->node_ptr(3)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(7) =
        mesh->node_ptr(elem->node_ptr(4)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(8) =
        mesh->node_ptr(elem->node_ptr(5)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(9) =
        mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(10) =
        mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(11) =
        mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(12) =
        mesh->node_ptr(elem->node_ptr(3)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(13) =
        mesh->node_ptr(elem->node_ptr(4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(14) =
        mesh->node_ptr(elem->node_ptr(5)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(15) =
        mesh->node_ptr(elem->node_ptr(3)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(16) =
        mesh->node_ptr(elem->node_ptr(4)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(17) =
        mesh->node_ptr(elem->node_ptr(5)->id() + ((current_layer * 2 + 1) * orig_nodes));
  }
  new_elem->set_node(0) =
      mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(1) =
      mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(2) =
      mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(3) =
      mesh->node_ptr(elem->node_ptr(0)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(4) =
      mesh->node_ptr(elem->node_ptr(1)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(5) =
      mesh->node_ptr(elem->node_ptr(2)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 0, 3);
    MooseMeshUtils::swapNodesInElem(*new_elem, 1, 4);
    MooseMeshUtils::swapNodesInElem(*new_elem, 2, 5);
    if (prism_elem_type != PRISM6)
    {
      MooseMeshUtils::swapNodesInElem(*new_elem, 6, 12);
      MooseMeshUtils::swapNodesInElem(*new_elem, 7, 13);
      MooseMeshUtils::swapNodesInElem(*new_elem, 8, 14);
      if (prism_elem_type == PRISM21)
      {
        MooseMeshUtils::swapNodesInElem(*new_elem, 18, 19);
      }
    }
    is_flipped = true;
  }
}

void
RevolveGenerator::createPYRAMIDfromTRI(
    const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
    const ElemType pyramid_elem_type,
    const Elem * elem,
    const std::unique_ptr<MeshBase> & mesh,
    std::unique_ptr<Elem> & new_elem,
    const int current_layer,
    const unsigned int orig_nodes,
    const unsigned int total_num_azimuthal_intervals,
    std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
    dof_id_type & axis_node_case,
    bool & is_flipped) const
{
  if (pyramid_elem_type != PYRAMID5 && pyramid_elem_type != PYRAMID13 &&
      pyramid_elem_type != PYRAMID18)
    mooseError("unsupported situation");

  side_pairs.push_back(std::make_pair(0, 2));
  const unsigned int order = pyramid_elem_type == PYRAMID5 ? 1 : 2;
  axis_node_case = nodes_cates.first.front();

  new_elem = std::make_unique<Pyramid5>();

  if (order == 2)
  {
    new_elem = std::make_unique<Pyramid13>();
    if (pyramid_elem_type == PYRAMID18)
    {
      new_elem = std::make_unique<Pyramid18>();
      new_elem->set_node(13) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 3 + 3)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(15) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 3 + 3)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(17) = mesh->node_ptr(elem->node_ptr(axis_node_case + 3)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(14) =
          mesh->node_ptr(elem->node_ptr(6)->id() + (current_layer * 2 * orig_nodes));
      new_elem->set_node(16) = mesh->node_ptr(
          elem->node_ptr(6)->id() +
          ((current_layer + 1) %
           (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) * 2 *
           orig_nodes));
    }
    new_elem->set_node(6) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 3)->id() +
                                           ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(8) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 3)->id() +
                                           ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(5) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 3 + 3)->id() +
                                           (current_layer * 2 * orig_nodes));
    new_elem->set_node(7) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 3 + 3)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(9) =
        mesh->node_ptr(elem->node_ptr(axis_node_case + 3)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(10) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 3 + 3)->id() +
                                            (current_layer * 2 * orig_nodes));
    new_elem->set_node(12) =
        mesh->node_ptr(elem->node_ptr(axis_node_case + 3)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(11) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 3 + 3)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
  }
  new_elem->set_node(4) = mesh->node_ptr(elem->node_ptr(axis_node_case)->id());
  new_elem->set_node(0) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 3)->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(1) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 3)->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(2) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 3)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(3) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 3)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 1, 2);
    MooseMeshUtils::swapNodesInElem(*new_elem, 0, 3);
    if (order == 2)
    {
      MooseMeshUtils::swapNodesInElem(*new_elem, 5, 7);
      MooseMeshUtils::swapNodesInElem(*new_elem, 10, 11);
      MooseMeshUtils::swapNodesInElem(*new_elem, 9, 12);
      if (pyramid_elem_type == PYRAMID18)
      {
        MooseMeshUtils::swapNodesInElem(*new_elem, 14, 16);
      }
    }
    is_flipped = true;
  }
}

void
RevolveGenerator::createTETfromTRI(
    const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
    const ElemType tet_elem_type,
    const Elem * elem,
    const std::unique_ptr<MeshBase> & mesh,
    std::unique_ptr<Elem> & new_elem,
    const int current_layer,
    const unsigned int orig_nodes,
    const unsigned int total_num_azimuthal_intervals,
    std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
    dof_id_type & axis_node_case,
    bool & is_flipped) const
{
  if (tet_elem_type != TET4 && tet_elem_type != TET10 && tet_elem_type != TET14)
    mooseError("unsupported situation");

  side_pairs.push_back(std::make_pair(0, 1));
  const unsigned int order = tet_elem_type == TET4 ? 1 : 2;
  if (order == 2)
  {
    // Sanity check to filter unsupported cases
    if (nodes_cates.first[0] > 2 || nodes_cates.first[1] > 2 || nodes_cates.first[2] < 3)
      mooseError("unsupported situation 2");
  }
  axis_node_case = nodes_cates.second.front();

  new_elem = std::make_unique<Tet4>();
  if (order == 2)
  {
    const bool node_order = nodes_cates.first[1] - nodes_cates.first[0] == 1;
    new_elem = std::make_unique<Tet10>();
    if (tet_elem_type == TET14)
    {
      new_elem = std::make_unique<Tet14>();
      new_elem->set_node(12) = mesh->node_ptr(
          elem->node_ptr(node_order ? (nodes_cates.first[1] + 3) : (nodes_cates.second.front() + 3))
              ->id() +
          ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(13) = mesh->node_ptr(
          elem->node_ptr(node_order ? (nodes_cates.second.front() + 3) : (nodes_cates.first[0] + 3))
              ->id() +
          ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(10) =
          mesh->node_ptr(elem->node_ptr(6)->id() + (current_layer * 2 * orig_nodes));
      new_elem->set_node(11) = mesh->node_ptr(
          elem->node_ptr(6)->id() +
          ((current_layer + 1) %
           (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) * 2 *
           orig_nodes));
    }
    new_elem->set_node(4) = mesh->node_ptr(
        elem->node_ptr(node_order ? (nodes_cates.first[0] + 3) : (nodes_cates.first[1] + 3))->id());
    new_elem->set_node(5) = mesh->node_ptr(
        elem->node_ptr(node_order ? (nodes_cates.first[1] + 3) : (nodes_cates.second.front() + 3))
            ->id() +
        (current_layer * 2 * orig_nodes));
    new_elem->set_node(6) = mesh->node_ptr(
        elem->node_ptr(node_order ? (nodes_cates.second.front() + 3) : (nodes_cates.first[0] + 3))
            ->id() +
        (current_layer * 2 * orig_nodes));
    new_elem->set_node(8) = mesh->node_ptr(
        elem->node_ptr(node_order ? (nodes_cates.first[1] + 3) : (nodes_cates.second.front() + 3))
            ->id() +
        ((current_layer + 1) %
         (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) * 2 *
         orig_nodes));
    new_elem->set_node(7) = mesh->node_ptr(
        elem->node_ptr(node_order ? (nodes_cates.second.front() + 3) : (nodes_cates.first[0] + 3))
            ->id() +
        ((current_layer + 1) %
         (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) * 2 *
         orig_nodes));
    new_elem->set_node(9) = mesh->node_ptr(elem->node_ptr(nodes_cates.second.front())->id() +
                                           ((current_layer * 2 + 1) * orig_nodes));
  }
  new_elem->set_node(0) = mesh->node_ptr(elem->node_ptr(nodes_cates.first[0])->id());
  new_elem->set_node(1) = mesh->node_ptr(elem->node_ptr(nodes_cates.first[1])->id());
  new_elem->set_node(2) = mesh->node_ptr(elem->node_ptr(nodes_cates.second.front())->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(3) =
      mesh->node_ptr(elem->node_ptr(nodes_cates.second.front())->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 2, 3);
    if (order == 2)
    {
      MooseMeshUtils::swapNodesInElem(*new_elem, 5, 8);
      MooseMeshUtils::swapNodesInElem(*new_elem, 6, 7);
      if (tet_elem_type == TET14)
      {
        MooseMeshUtils::swapNodesInElem(*new_elem, 10, 11);
      }
    }
    is_flipped = true;
  }
}

void
RevolveGenerator::createHEXfromQUAD(const ElemType hex_elem_type,
                                    const Elem * elem,
                                    const std::unique_ptr<MeshBase> & mesh,
                                    std::unique_ptr<Elem> & new_elem,
                                    const int current_layer,
                                    const unsigned int orig_nodes,
                                    const unsigned int total_num_azimuthal_intervals,
                                    std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
                                    bool & is_flipped) const
{
  if (hex_elem_type != HEX8 && hex_elem_type != HEX20 && hex_elem_type != HEX27)
    mooseError("unsupported situation");

  side_pairs.push_back(std::make_pair(0, 5));
  const unsigned int order = hex_elem_type == HEX8 ? 1 : 2;

  new_elem = std::make_unique<Hex8>();
  if (order == 2)
  {
    new_elem = std::make_unique<Hex20>();
    if (hex_elem_type == HEX27)
    {
      new_elem = std::make_unique<Hex27>();
      new_elem->set_node(20) =
          mesh->node_ptr(elem->node_ptr(8)->id() + (current_layer * 2 * orig_nodes));
      new_elem->set_node(25) = mesh->node_ptr(
          elem->node_ptr(8)->id() +
          ((current_layer + 1) %
           (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) * 2 *
           orig_nodes));
      new_elem->set_node(26) =
          mesh->node_ptr(elem->node_ptr(8)->id() + ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(21) =
          mesh->node_ptr(elem->node_ptr(4)->id() + ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(22) =
          mesh->node_ptr(elem->node_ptr(5)->id() + ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(23) =
          mesh->node_ptr(elem->node_ptr(6)->id() + ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(24) =
          mesh->node_ptr(elem->node_ptr(7)->id() + ((current_layer * 2 + 1) * orig_nodes));
    }
    new_elem->set_node(8) =
        mesh->node_ptr(elem->node_ptr(4)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(9) =
        mesh->node_ptr(elem->node_ptr(5)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(10) =
        mesh->node_ptr(elem->node_ptr(6)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(11) =
        mesh->node_ptr(elem->node_ptr(7)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(16) =
        mesh->node_ptr(elem->node_ptr(4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(17) =
        mesh->node_ptr(elem->node_ptr(5)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(18) =
        mesh->node_ptr(elem->node_ptr(6)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(19) =
        mesh->node_ptr(elem->node_ptr(7)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(12) =
        mesh->node_ptr(elem->node_ptr(0)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(13) =
        mesh->node_ptr(elem->node_ptr(1)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(14) =
        mesh->node_ptr(elem->node_ptr(2)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(15) =
        mesh->node_ptr(elem->node_ptr(3)->id() + ((current_layer * 2 + 1) * orig_nodes));
  }
  new_elem->set_node(0) =
      mesh->node_ptr(elem->node_ptr(0)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(1) =
      mesh->node_ptr(elem->node_ptr(1)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(2) =
      mesh->node_ptr(elem->node_ptr(2)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(3) =
      mesh->node_ptr(elem->node_ptr(3)->id() + (current_layer * order * orig_nodes));
  new_elem->set_node(4) =
      mesh->node_ptr(elem->node_ptr(0)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(5) =
      mesh->node_ptr(elem->node_ptr(1)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(6) =
      mesh->node_ptr(elem->node_ptr(2)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(7) =
      mesh->node_ptr(elem->node_ptr(3)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 0, 4);
    MooseMeshUtils::swapNodesInElem(*new_elem, 1, 5);
    MooseMeshUtils::swapNodesInElem(*new_elem, 2, 6);
    MooseMeshUtils::swapNodesInElem(*new_elem, 3, 7);
    if (order == 2)
    {
      MooseMeshUtils::swapNodesInElem(*new_elem, 8, 16);
      MooseMeshUtils::swapNodesInElem(*new_elem, 9, 17);
      MooseMeshUtils::swapNodesInElem(*new_elem, 10, 18);
      MooseMeshUtils::swapNodesInElem(*new_elem, 11, 19);
      if (hex_elem_type == HEX27)
      {
        MooseMeshUtils::swapNodesInElem(*new_elem, 20, 25);
      }
    }
    is_flipped = true;
  }
}

void
RevolveGenerator::createPRISMfromQUAD(
    const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
    const ElemType prism_elem_type,
    const Elem * elem,
    const std::unique_ptr<MeshBase> & mesh,
    std::unique_ptr<Elem> & new_elem,
    const int current_layer,
    const unsigned int orig_nodes,
    const unsigned int total_num_azimuthal_intervals,
    std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
    dof_id_type & axis_node_case,
    bool & is_flipped) const
{
  if (prism_elem_type != PRISM6 && prism_elem_type != PRISM15 && prism_elem_type != PRISM18)
    mooseError("unsupported situation");

  side_pairs.push_back(std::make_pair(1, 3));
  const unsigned int order = prism_elem_type == PRISM6 ? 1 : 2;
  if (order == 2)
  {
    // Sanity check to filter unsupported cases
    if (nodes_cates.first[0] > 3 || nodes_cates.first[1] > 3 || nodes_cates.first[2] < 4)
      mooseError("unsupported situation 2");
  }
  // Can only be 0-1, 1-2, 2-3, 3-0, we only consider vetices here.
  // nodes_cates are natually sorted
  const dof_id_type min_on_axis = nodes_cates.first[0];
  const dof_id_type max_on_axis = nodes_cates.first[1];
  axis_node_case = max_on_axis - min_on_axis == 1 ? min_on_axis : max_on_axis;

  new_elem = std::make_unique<Prism6>();
  if (order == 2)
  {
    new_elem = std::make_unique<Prism15>();
    if (prism_elem_type == PRISM18)
    {
      new_elem = std::make_unique<Prism18>();
      new_elem->set_node(15) =
          mesh->node_ptr(elem->node_ptr(8)->id() + (current_layer * 2 * orig_nodes));
      new_elem->set_node(16) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4 + 4)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
      new_elem->set_node(17) = mesh->node_ptr(
          elem->node_ptr(8)->id() +
          ((current_layer + 1) %
           (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) * 2 *
           orig_nodes));
    }
    new_elem->set_node(9) = mesh->node_ptr(elem->node_ptr(axis_node_case + 4)->id());
    new_elem->set_node(10) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4 + 4)->id() +
                                            (current_layer * 2 * orig_nodes));
    new_elem->set_node(12) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4 + 4)->id() +
                                            (current_layer * 2 * orig_nodes));
    new_elem->set_node(6) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4 + 4)->id() +
                                           (current_layer * 2 * orig_nodes));
    new_elem->set_node(14) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4 + 4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(8) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4 + 4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(11) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4 + 4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(7) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                                           ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(13) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4)->id() +
                                            ((current_layer * 2 + 1) * orig_nodes));
  }
  new_elem->set_node(0) = mesh->node_ptr(elem->node_ptr(axis_node_case)->id());
  new_elem->set_node(3) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4)->id());
  new_elem->set_node(4) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4)->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(1) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(5) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(2) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 1, 2);
    MooseMeshUtils::swapNodesInElem(*new_elem, 4, 5);
    if (order == 2)
    {
      MooseMeshUtils::swapNodesInElem(*new_elem, 12, 14);
      MooseMeshUtils::swapNodesInElem(*new_elem, 6, 8);
      MooseMeshUtils::swapNodesInElem(*new_elem, 10, 11);
      if (prism_elem_type == PRISM18)
      {
        MooseMeshUtils::swapNodesInElem(*new_elem, 15, 17);
      }
    }
    is_flipped = true;
  }
}

void
RevolveGenerator::createPYRAMIDPRISMfromQUAD(
    const std::pair<std::vector<dof_id_type>, std::vector<dof_id_type>> & nodes_cates,
    const ElemType pyramid_elem_type,
    const ElemType prism_elem_type,
    const Elem * elem,
    const std::unique_ptr<MeshBase> & mesh,
    std::unique_ptr<Elem> & new_elem,
    std::unique_ptr<Elem> & new_elem_1,
    const int current_layer,
    const unsigned int orig_nodes,
    const unsigned int total_num_azimuthal_intervals,
    std::vector<std::pair<dof_id_type, dof_id_type>> & side_pairs,
    dof_id_type & axis_node_case,
    bool & is_flipped,
    bool & is_flipped_additional) const
{
  if (!(pyramid_elem_type == PYRAMID5 && prism_elem_type == PRISM6) &&
      !(pyramid_elem_type == PYRAMID14 && prism_elem_type == PRISM18))
    mooseError("unsupported situation");
  const unsigned int order = pyramid_elem_type == PYRAMID5 ? 1 : 2;

  side_pairs.push_back(std::make_pair(0, 2));
  axis_node_case = nodes_cates.first.front();
  new_elem = std::make_unique<Pyramid5>();
  if (pyramid_elem_type == PYRAMID14)
  {
    new_elem = std::make_unique<Pyramid14>();
    new_elem->set_node(9) =
        mesh->node_ptr(elem->node_ptr(axis_node_case + 4)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(10) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4 + 4)->id() +
                                            (current_layer * 2 * orig_nodes));
    new_elem->set_node(5) =
        mesh->node_ptr(elem->node_ptr(8)->id() + (current_layer * 2 * orig_nodes));
    new_elem->set_node(8) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4)->id() +
                                           ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(6) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                                           ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(13) =
        mesh->node_ptr(elem->node_ptr(8)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem->set_node(7) =
        mesh->node_ptr(elem->node_ptr(8)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(11) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4 + 4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem->set_node(12) =
        mesh->node_ptr(elem->node_ptr(axis_node_case + 4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
  }
  new_elem->set_node(4) = mesh->node_ptr(elem->node_ptr(axis_node_case)->id());
  new_elem->set_node(0) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4)->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(1) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                                         (current_layer * order * orig_nodes));
  new_elem->set_node(2) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem->set_node(3) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem, 1, 2);
    MooseMeshUtils::swapNodesInElem(*new_elem, 0, 3);
    if (pyramid_elem_type == PYRAMID14)
    {
      MooseMeshUtils::swapNodesInElem(*new_elem, 5, 7);
      MooseMeshUtils::swapNodesInElem(*new_elem, 10, 11);
      MooseMeshUtils::swapNodesInElem(*new_elem, 9, 12);
    }
    is_flipped = true;
  }

  side_pairs.push_back(std::make_pair(0, 4));
  new_elem_1 = std::make_unique<Prism6>();
  if (prism_elem_type == PRISM18)
  {
    new_elem_1 = std::make_unique<Prism18>();
    new_elem_1->set_node(6) =
        mesh->node_ptr(elem->node_ptr(8)->id() + (current_layer * 2 * orig_nodes));
    new_elem_1->set_node(8) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4 + 4)->id() +
                                             (current_layer * 2 * orig_nodes));
    new_elem_1->set_node(7) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4 + 4)->id() +
                                             (current_layer * 2 * orig_nodes));
    new_elem_1->set_node(9) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4)->id() +
                                             ((current_layer * 2 + 1) * orig_nodes));
    new_elem_1->set_node(10) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
    new_elem_1->set_node(11) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
    new_elem_1->set_node(15) =
        mesh->node_ptr(elem->node_ptr(8)->id() + ((current_layer * 2 + 1) * orig_nodes));
    new_elem_1->set_node(17) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4 + 4)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
    new_elem_1->set_node(16) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4 + 4)->id() +
                                              ((current_layer * 2 + 1) * orig_nodes));
    new_elem_1->set_node(12) =
        mesh->node_ptr(elem->node_ptr(8)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem_1->set_node(13) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4 + 4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
    new_elem_1->set_node(14) =
        mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4 + 4)->id() +
                       ((current_layer + 1) %
                        (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                        2 * orig_nodes));
  }
  new_elem_1->set_node(0) = mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4)->id() +
                                           (current_layer * order * orig_nodes));
  new_elem_1->set_node(1) = mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                                           (current_layer * order * orig_nodes));
  new_elem_1->set_node(2) = mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4)->id() +
                                           (current_layer * order * orig_nodes));
  new_elem_1->set_node(3) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 1) % 4)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem_1->set_node(4) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 3) % 4)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));
  new_elem_1->set_node(5) =
      mesh->node_ptr(elem->node_ptr((axis_node_case + 2) % 4)->id() +
                     ((current_layer + 1) %
                      (total_num_azimuthal_intervals + 1 - (unsigned int)_full_circle_revolving) *
                      order * orig_nodes));

  if (new_elem_1->volume() < 0.0)
  {
    MooseMeshUtils::swapNodesInElem(*new_elem_1, 0, 3);
    MooseMeshUtils::swapNodesInElem(*new_elem_1, 1, 4);
    MooseMeshUtils::swapNodesInElem(*new_elem_1, 2, 5);
    if (prism_elem_type == PRISM18)
    {
      MooseMeshUtils::swapNodesInElem(*new_elem_1, 6, 12);
      MooseMeshUtils::swapNodesInElem(*new_elem_1, 7, 13);
      MooseMeshUtils::swapNodesInElem(*new_elem_1, 8, 14);
    }
    is_flipped_additional = true;
  }
}
