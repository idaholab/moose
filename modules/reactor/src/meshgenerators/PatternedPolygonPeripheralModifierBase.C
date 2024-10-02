//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatternedPolygonPeripheralModifierBase.h"

#include "MooseMeshUtils.h"
#include "FillBetweenPointVectorsTools.h"
#include "KDTree.h"

InputParameters
PatternedPolygonPeripheralModifierBase::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>(
      "input",
      "The input mesh to be modified. Note that this generator only works with "
      "PatternedHex/CartesianMeshGenerator and its derived classes such as "
      "HexIDPatternedMeshGenerator.");
  params.addRequiredParam<BoundaryName>("input_mesh_external_boundary",
                                        "The external boundary of the input mesh.");
  params.addRequiredParam<unsigned int>("new_num_sector",
                                        "Number of sectors of each side for the new mesh.");
  params.addParam<subdomain_id_type>(
      "transition_layer_id",
      TRANSITION_LAYER_DEFAULT,
      "Optional customized block id for the transition layer block.");
  params.addParam<SubdomainName>("transition_layer_name",
                                 "transition_layer",
                                 "Optional customized block name for the transition layer block.");
  params.addRangeCheckedParam<unsigned int>(
      "num_layers", 1, "num_layers>0", "Layers of elements for transition.");
  params.addParam<std::vector<std::string>>(
      "extra_id_names_to_modify",
      "Names of the element extra ids in the peripheral region that should be modified");
  params.addParam<std::vector<dof_id_type>>(
      "new_extra_id_values_to_assign",
      std::vector<dof_id_type>(),
      "Values of the modified extra ids in the peripheral region.");
  params.addClassDescription("PatternedPolygonPeripheralModifierBase is the base class for "
                             "PatternedCartPeripheralModifier and PatternedHexPeripheralModifier.");

  return params;
}

PatternedPolygonPeripheralModifierBase::PatternedPolygonPeripheralModifierBase(
    const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _input_mesh_external_boundary(getParam<BoundaryName>("input_mesh_external_boundary")),
    _new_num_sector(getParam<unsigned int>("new_num_sector")),
    _transition_layer_id(getParam<subdomain_id_type>("transition_layer_id")),
    _transition_layer_name(getParam<SubdomainName>("transition_layer_name")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _extra_id_names_to_modify(isParamValid("extra_id_names_to_modify")
                                  ? getParam<std::vector<std::string>>("extra_id_names_to_modify")
                                  : std::vector<std::string>()),
    _new_extra_id_values_to_assign(
        getParam<std::vector<dof_id_type>>("new_extra_id_values_to_assign")),
    // Use CartesianConcentricCircleAdaptiveBoundaryMeshGenerator for cartesian control drum meshing
    // Use HexagonConcentricCircleAdaptiveBoundaryMeshGenerator for hexagonal control drum meshing
    _mesh(getMeshByName(_input_name))
{
  declareMeshProperty<Real>("pattern_pitch_meta", 0.0);
  declareMeshProperty<bool>("is_control_drum_meta", false);
  if (_extra_id_names_to_modify.size() != _new_extra_id_values_to_assign.size())
    paramError("new_extra_id_values_to_assign",
               "This parameter must have the same size as extra_id_names_to_modify.");
  declareMeshProperty<bool>("peripheral_modifier_compatible", false);
}

std::unique_ptr<MeshBase>
PatternedPolygonPeripheralModifierBase::generate()
{
  // Transmit the Mesh Metadata
  auto pattern_pitch_meta = setMeshProperty(
      "pattern_pitch_meta", getMeshProperty<Real>("pattern_pitch_meta", _input_name));
  // Load the input mesh as ReplicatedMesh
  auto input_mesh = dynamic_pointer_cast<ReplicatedMesh>(_mesh);
  // Load boundary information
  _input_mesh_external_bid =
      MooseMeshUtils::getBoundaryID(_input_mesh_external_boundary, *input_mesh);
  if (_input_mesh_external_bid == Moose::INVALID_BOUNDARY_ID)
    paramError("input_mesh_external_boundary",
               "External boundary does not exist in the input mesh");
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> side_list =
      input_mesh->get_boundary_info().build_side_list();
  input_mesh->get_boundary_info().build_node_list_from_side_list();
  std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list =
      input_mesh->get_boundary_info().build_node_list();
  // Find neighbors
  if (!input_mesh->is_prepared())
    input_mesh->find_neighbors();
  // Load all the extra integer indexing information from the input mesh
  const unsigned int n_extra_integer_input = input_mesh->n_elem_integers();
  std::vector<std::string> extra_integer_names;
  for (unsigned int i = 0; i < n_extra_integer_input; i++)
    extra_integer_names.push_back(input_mesh->get_elem_integer_name(i));

  // Check elem extra integers names and convert them into indices
  std::vector<bool> extra_id_modify_flags = std::vector<bool>(n_extra_integer_input, false);
  std::vector<dof_id_type> extra_modify_values = std::vector<dof_id_type>(n_extra_integer_input, 0);
  for (unsigned int i = 0; i < _extra_id_names_to_modify.size(); i++)
  {
    if (!input_mesh->has_elem_integer(_extra_id_names_to_modify[i]))
      paramError(
          "extra_id_names_to_modify",
          "The parameter contains an extra element integer that does not exist in the input mesh.");
    else
    {
      extra_id_modify_flags[input_mesh->get_elem_integer_index(_extra_id_names_to_modify[i])] =
          true;
      extra_modify_values[input_mesh->get_elem_integer_index(_extra_id_names_to_modify[i])] =
          _new_extra_id_values_to_assign[i];
    }
  }

  std::vector<dof_id_type> bid_elem_list;
  std::vector<dof_id_type> new_bid_elem_list;
  std::vector<dof_id_type> ext_node_list;
  // We check the type of the QUAD elements on the boundary
  unsigned short quad_type = 0;
  for (unsigned int i = 0; i < side_list.size(); i++)
  {
    if (std::get<2>(side_list[i]) == _input_mesh_external_bid)
    {
      // List of elements on the boundary
      bid_elem_list.push_back(std::get<0>(side_list[i]));
      // Check if the element is QUAD4 or QUAD8/9
      const auto elem_type = input_mesh->elem_ptr(std::get<0>(side_list[i]))->type();
      if (elem_type != QUAD4 && elem_type != QUAD8 && elem_type != QUAD9)
        paramError("input",
                   "The input mesh has non-QUAD4/QUAD8/QUAD9 elements in its peripheral area, "
                   "which is not supported.");
      // Generally, we need to check if the sideset involves mixed order elements
      // But this is not possible for a mesh generated by PatternedHex/CartesianMeshGenerator
      // So we only need to check the first element
      else if (quad_type == 0)
        quad_type = elem_type == QUAD4 ? 1 : (elem_type == QUAD8 ? 8 : 9);

      // Note this object only works with quad elements
      // (side_id + 2)%4 gives the opposite side's neighboring element
      // Thus, list of elements on the new boundary is made
      new_bid_elem_list.push_back(input_mesh->elem_ptr(std::get<0>(side_list[i]))
                                      ->neighbor_ptr((std::get<1>(side_list[i]) + 2) % 4)
                                      ->id());
    }
  }
  // This eliminates the duplicate elements, which happen at the vertices.
  auto bid_elem_list_it = std::unique(bid_elem_list.begin(), bid_elem_list.end());
  auto new_bid_elem_list_it = std::unique(new_bid_elem_list.begin(), new_bid_elem_list.end());
  bid_elem_list.resize(std::distance(bid_elem_list.begin(), bid_elem_list_it));
  new_bid_elem_list.resize(std::distance(new_bid_elem_list.begin(), new_bid_elem_list_it));
  // Create a data structure to record centroid positions and extra element IDS of the
  // elements to be deleted
  std::vector<std::pair<Point, std::vector<dof_id_type>>> del_elem_extra_ids;
  // Delete the outer layer QUAD elements so that they can be replaced by TRI elements later.
  for (unsigned int i = 0; i < bid_elem_list.size(); i++)
  {
    const auto & tmp_elem_ptr = input_mesh->elem_ptr(bid_elem_list[i]);
    // Retain all the extra integer information first if any of them need to be retained
    if (n_extra_integer_input > _extra_id_names_to_modify.size())
    {
      del_elem_extra_ids.push_back(
          std::make_pair(tmp_elem_ptr->true_centroid(), std::vector<dof_id_type>()));
      for (unsigned int j = 0; j < n_extra_integer_input; j++)
        del_elem_extra_ids[i].second.push_back(
            extra_id_modify_flags[j] ? extra_modify_values[j] : tmp_elem_ptr->get_extra_integer(j));
    }
    input_mesh->delete_elem(tmp_elem_ptr);
  }
  // if all the extra id will need to be re-assigned, just a dummy reference vector is needed.
  if (n_extra_integer_input == _extra_id_names_to_modify.size())
    del_elem_extra_ids.push_back(
        std::make_pair(Point(0.0, 0.0, 0.0), _new_extra_id_values_to_assign));
  // As some elements are deleted, update the neighbor list
  input_mesh->find_neighbors();
  // Identify the new external boundary
  BoundaryInfo & boundary_info = input_mesh->get_boundary_info();
  for (unsigned int i = 0; i < new_bid_elem_list.size(); i++)
  {
    // Assign default external Sideset ID to the new boundary
    for (unsigned int j = 0; j < input_mesh->elem_ptr(new_bid_elem_list[i])->n_sides(); j++)
    {
      if (input_mesh->elem_ptr(new_bid_elem_list[i])->neighbor_ptr(j) == nullptr)
        boundary_info.add_side(new_bid_elem_list[i], j, OUTER_SIDESET_ID);
    }
  }

  // Refresh the mesh as it lost some elements
  input_mesh->contract();
  input_mesh->prepare_for_use();

  // Clone the original mesh without outer layer for information extraction
  auto input_mesh_origin = dynamic_pointer_cast<ReplicatedMesh>(input_mesh->clone());

  // Make four (cartesian) or six (hexagonal) meshes of new outer layer for four or six sides.
  for (unsigned i_side = 0; i_side < _num_sides; i_side++)
  {
    // Use azimuthalAnglesCollector to assign boundary Points to a reference vector
    std::vector<Point> inner_pts;
    std::vector<Real> new_bdry_azi = azimuthalAnglesCollector(*input_mesh_origin,
                                                              inner_pts,
                                                              -180.0 / (Real)_num_sides,
                                                              180.0 / (Real)_num_sides,
                                                              ANGLE_DEGREE);
    MeshTools::Modification::rotate(*input_mesh_origin, -360.0 / (Real)_num_sides, 0, 0);

    // Setting up of the outer boundary is done by defining the two ends
    // And then use libMesh interpolation tool to make the straight line
    std::vector<Point> outer_pts;
    const Point outer_end_1 = Point(pattern_pitch_meta / 2.0,
                                    -pattern_pitch_meta / 2.0 * std::tan(M_PI / (Real)_num_sides),
                                    0.0);
    const Point outer_end_2 = Point(pattern_pitch_meta / 2.0,
                                    pattern_pitch_meta / 2.0 * std::tan(M_PI / (Real)_num_sides),
                                    0.0);

    const std::vector<Real> input_arg{0.0, 1.0};
    const std::vector<Real> input_x{outer_end_1(0), outer_end_2(0)};
    const std::vector<Real> input_y{outer_end_1(1), outer_end_2(1)};
    std::unique_ptr<LinearInterpolation> linear_outer_x =
        std::make_unique<LinearInterpolation>(input_arg, input_x);
    std::unique_ptr<LinearInterpolation> linear_outer_y =
        std::make_unique<LinearInterpolation>(input_arg, input_y);

    // Uniformly spaced nodes on the outer boundary to facilitate stitching
    for (unsigned int i = 0; i < _new_num_sector + 1; i++)
    {
      outer_pts.push_back(Point(linear_outer_x->sample((Real)i / (Real)_new_num_sector),
                                linear_outer_y->sample((Real)i / (Real)_new_num_sector),
                                0.0));
    }
    // Now we have the inner and outer boundary points as input for the transition layer
    // For linear elements, that's all we need;
    // For quadratic elements, the inner point vector contains both vertices and midpoints.
    // fillBetweenPointVectorsGenerator only supports linear elements
    // So we need to
    // (1) remove the midpoints from the inner point vector
    // (2) generate linear mesh using fillBetweenPointVectorsGenerator
    // (3) convert the linear mesh to quadratic mesh
    // Generally, in the input mesh, the midpoint might not be located in the middle of the two
    // vertices. (4) In that case, the midpoint needs to be moved to match the input mesh In the
    // case of PatternedHex/CartesianMeshGenerator, the midpoints of the inner boundary shoule be
    // the average of the vertices unless the non-circular regions are deformed. So this step still
    // needs to be done.

    // Use fillBetweenPointVectorsGenerator to generate the transition layer

    // Remove the midpoints from the inner point vector
    // But record the midpoints for later use
    // In the meantime, calculate the average of the two vertices
    std::vector<std::pair<Point, Point>> inner_midpts_pairs;
    if (quad_type > 4)
    {
      auto inner_pts_tmp(inner_pts);
      inner_pts.clear();
      for (const auto i : index_range(inner_pts_tmp))
        if (i % 2 == 0)
          inner_pts.push_back(inner_pts_tmp[i]);
        else
        {
          // We check if the midpoint is located in the middle of the two vertices
          // If not, we record the them for later use
          const Point avgpt = 0.5 * (inner_pts_tmp[i - 1] + inner_pts_tmp[i + 1]);
          if (!MooseUtils::absoluteFuzzyEqual((avgpt - inner_pts_tmp[i]).norm(), 0.0))
            inner_midpts_pairs.push_back(std::make_pair(avgpt, inner_pts_tmp[i]));
        }
      inner_pts_tmp.clear();
    }

    auto mesh = buildReplicatedMesh(2);
    FillBetweenPointVectorsTools::fillBetweenPointVectorsGenerator(*mesh,
                                                                   inner_pts,
                                                                   outer_pts,
                                                                   _num_layers,
                                                                   _transition_layer_id,
                                                                   OUTER_SIDESET_ID,
                                                                   _type,
                                                                   _name);

    // Convert the linear mesh to quadratic mesh
    if (quad_type == 8)
      mesh->all_second_order();
    else if (quad_type == 9)
      mesh->all_complete_order();

    // Move the midpoints to match the input mesh if applicable
    if (inner_midpts_pairs.size())
    {
      mesh->get_boundary_info().build_node_list_from_side_list();
      auto tl_node_list = mesh->get_boundary_info().build_node_list();
      std::vector<dof_id_type> bdry_node_ids;
      std::vector<Point> bdry_node_pts;
      for (const auto & tl_node_tuple : tl_node_list)
      {
        if (std::get<1>(tl_node_tuple) == OUTER_SIDESET_ID)
        {
          bdry_node_ids.push_back(std::get<0>(tl_node_tuple));
          bdry_node_pts.push_back(*(mesh->node_ptr(std::get<0>(tl_node_tuple))));
        }
      }
      KDTree ref_kd_tree(bdry_node_pts, 4);
      for (const auto & midpt_pair : inner_midpts_pairs)
      {
        std::vector<std::size_t> nn_id;
        ref_kd_tree.neighborSearch(midpt_pair.first, 1, nn_id);
        *(mesh->node_ptr(bdry_node_ids[nn_id.front()])) = midpt_pair.second;
      }
    }

    MeshTools::Modification::rotate(*mesh, (Real)i_side * 360.0 / (Real)_num_sides, 0, 0);
    // Assign extra element id based on the nearest deleted element
    mesh->add_elem_integers(extra_integer_names, true);
    transferExtraElemIntegers(*mesh, del_elem_extra_ids);
    mesh->prepare_for_use();
    input_mesh->stitch_meshes(*mesh, OUTER_SIDESET_ID, OUTER_SIDESET_ID, TOLERANCE, true);
    mesh->clear();
  }
  input_mesh->subdomain_name(_transition_layer_id) = _transition_layer_name;
  return input_mesh;
}

void
PatternedPolygonPeripheralModifierBase::transferExtraElemIntegers(
    ReplicatedMesh & mesh,
    const std::vector<std::pair<Point, std::vector<dof_id_type>>> ref_extra_ids)
{
  // Build points vector for k-d tree constructor
  std::vector<Point> ref_pts;
  for (auto & pt_extra_id : ref_extra_ids)
    ref_pts.push_back(pt_extra_id.first);
  // K-d tree construction
  KDTree ref_kd_tree(ref_pts, 4);
  // Use the k-d tree for nearest neighbor searching
  for (const auto & elem : as_range(mesh.active_elements_begin(), mesh.active_elements_end()))
  {
    std::vector<std::size_t> nn_id;
    ref_kd_tree.neighborSearch(elem->true_centroid(), 1, nn_id);
    for (unsigned int i = 0; i < ref_extra_ids[nn_id.front()].second.size(); i++)
      elem->set_extra_integer(i, ref_extra_ids[nn_id.front()].second[i]);
  }
}
