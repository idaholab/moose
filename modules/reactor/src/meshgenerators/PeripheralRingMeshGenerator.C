//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeripheralRingMeshGenerator.h"

#include "MooseMeshUtils.h"

// C++ includes
#include <cmath> // for atan2

registerMooseObject("ReactorApp", PeripheralRingMeshGenerator);

InputParameters
PeripheralRingMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to be modified.");
  params.addRangeCheckedParam<unsigned int>(
      "peripheral_layer_num",
      3,
      "peripheral_layer_num>0",
      "The radial layers of the peripheral ring to be added.");
  params.addRequiredRangeCheckedParam<Real>("peripheral_ring_radius",
                                            "peripheral_ring_radius>0",
                                            "Radius of the peripheral ring to be added.");
  params.addParam<bool>(
      "preserve_volumes",
      true,
      "Whether the volume of the peripheral region is preserved by fixing the radius.");
  params.addRequiredParam<BoundaryName>("input_mesh_external_boundary",
                                        "The external boundary of the input mesh.");
  params.addRequiredParam<subdomain_id_type>(
      "peripheral_ring_block_id", "The block id assigned to the created peripheral layer.");
  params.addParam<SubdomainName>("peripheral_ring_block_name",
                                 "The block name assigned to the created peripheral layer.");
  params.addRangeCheckedParam<boundary_id_type>("external_boundary_id",
                                                "external_boundary_id>0",
                                                "Optional customized external boundary id.");
  params.addParam<std::string>("external_boundary_name",
                               "Optional customized external boundary name.");
  params.addClassDescription("This PeripheralRingMeshGenerator object adds a circular peripheral "
                             "region to the input mesh.");

  return params;
}

PeripheralRingMeshGenerator::PeripheralRingMeshGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _peripheral_layer_num(getParam<unsigned int>("peripheral_layer_num")),
    _peripheral_ring_radius(getParam<Real>("peripheral_ring_radius")),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _input_mesh_external_boundary(getParam<BoundaryName>("input_mesh_external_boundary")),
    _peripheral_ring_block_id(getParam<subdomain_id_type>("peripheral_ring_block_id")),
    _peripheral_ring_block_name(isParamValid("peripheral_ring_block_name")
                                    ? getParam<SubdomainName>("peripheral_ring_block_name")
                                    : (SubdomainName) ""),
    _external_boundary_id(isParamValid("external_boundary_id")
                              ? getParam<boundary_id_type>("external_boundary_id")
                              : 0),
    _external_boundary_name(isParamValid("external_boundary_name")
                                ? getParam<std::string>("external_boundary_name")
                                : std::string()),
    _input(getMeshByName(_input_name))
{
}

std::unique_ptr<MeshBase>
PeripheralRingMeshGenerator::generate()
{
  // Need ReplicatedMesh for stitching
  auto input_mesh = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!input_mesh)
    paramError("input", "Input is not a replicated mesh, which is required.");
  if (*(input_mesh->elem_dimensions().begin()) != 2 ||
      *(input_mesh->elem_dimensions().rbegin()) != 2)
    paramError("input", "Only 2D meshes are supported.");
  _input_mesh_external_bid =
      MooseMeshUtils::getBoundaryID(_input_mesh_external_boundary, *input_mesh);

  // Use CoM of the input mesh as its origin for azimuthal calculation
  const Point origin_pt = MooseMeshUtils::meshCentroidCalculator(*input_mesh);
  const Real origin_x = origin_pt(0);
  const Real origin_y = origin_pt(1);
  const Real origin_z = origin_pt(2);
  // Vessel for containing maximum radius of the boundary nodes
  Real max_input_mesh_node_radius;
  unsigned short invalid_boundary_type;

  if (!isBoundaryValid(*input_mesh,
                       max_input_mesh_node_radius,
                       invalid_boundary_type,
                       origin_pt,
                       _input_mesh_external_bid))
  {
    switch (invalid_boundary_type)
    {
      case 1:
        paramError("input_mesh_external_boundary",
                   "This mesh generator does not work for the provided external boundary as it has "
                   "more than one segments.");
        break;
      case 2:
        paramError("input_mesh_external_boundary",
                   "This mesh generator does not work for the provided external boundary as it is "
                   "not a closed loop.");
        break;
      case 3:
        paramError("input_mesh_external_boundary",
                   "This mesh generator does not work for the provided external boundary as "
                   "azimuthal angles of consecutive nodes do not change monotonically.");
        break;
    }
  }

  if (max_input_mesh_node_radius >= _peripheral_ring_radius)
    paramError(
        "peripheral_ring_radius",
        "The peripheral ring to be generated must be large enough to cover the entire input mesh.");
  if (!isExternalBoundary(*input_mesh, _input_mesh_external_bid))
    paramError("input_mesh_external_boundary",
               "The boundary provided is not an external boundary.");

  std::vector<Point> input_ext_bd_pts;
  std::vector<Point> output_ext_bd_pts;
  std::vector<std::tuple<Real, Point, Point>> azi_points;
  std::vector<Real> azi_array;
  Real tmp_azi(0.0);
  auto mesh = buildReplicatedMesh(2);

  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> side_list =
      input_mesh->get_boundary_info().build_side_list();
  input_mesh->get_boundary_info().build_node_list_from_side_list();
  std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list =
      input_mesh->get_boundary_info().build_node_list();

  // Node counter for the external boundary
  unsigned int input_ext_node_num = 0;

  // Loop over all the boundary nodes
  for (unsigned int i = 0; i < node_list.size(); ++i)
    if (std::get<1>(node_list[i]) == _input_mesh_external_bid)
    {
      input_ext_node_num++;
      // Define nodes on the inner and outer boundaries of the peripheral region.
      input_ext_bd_pts.push_back(input_mesh->point(std::get<0>(node_list[i])));
      tmp_azi = atan2(input_ext_bd_pts.back()(1) - origin_y, input_ext_bd_pts.back()(0) - origin_x);
      output_ext_bd_pts.push_back(Point(_peripheral_ring_radius * std::cos(tmp_azi),
                                        _peripheral_ring_radius * std::sin(tmp_azi),
                                        origin_z));
      // a vector of tuples using azimuthal angle as the key to facilitate sorting
      azi_points.push_back(
          std::make_tuple(tmp_azi, input_ext_bd_pts.back(), output_ext_bd_pts.back()));
      // a simple vector of azimuthal angles for radius correction purposes (in degree)
      azi_array.push_back(tmp_azi / M_PI * 180.0);
    }
  std::sort(azi_points.begin(), azi_points.end());
  std::sort(azi_array.begin(), azi_array.end());
  const Real correction_factor = _preserve_volumes ? radiusCorrectionFactor(azi_array) : 1.0;
  // 2D vector containing all the node positions of the peripheral region
  std::vector<std::vector<Point>> points_array(_peripheral_layer_num + 1,
                                               std::vector<Point>(input_ext_node_num));
  // 2D vector containing all the node ids of the peripheral region
  std::vector<std::vector<dof_id_type>> node_id_array(_peripheral_layer_num + 1,
                                                      std::vector<dof_id_type>(input_ext_node_num));
  for (unsigned int i = 0; i < input_ext_node_num; ++i)
  {
    // Inner boundary nodes of the peripheral region
    points_array[0][i] = std::get<1>(azi_points[i]);
    // Outer boundary nodes of the peripheral region
    points_array[_peripheral_layer_num][i] = std::get<2>(azi_points[i]) * correction_factor;
    // Use interpolation to get intermediate layers nodes
    for (unsigned int j = 1; j < _peripheral_layer_num; ++j)
      points_array[j][i] = (points_array[0][i] * ((Real)_peripheral_layer_num - (Real)j) +
                            points_array[_peripheral_layer_num][i] * (Real)j) /
                           (Real)_peripheral_layer_num;
  }
  unsigned int num_total_nodes = (_peripheral_layer_num + 1) * input_ext_node_num;
  std::vector<Node *> nodes(num_total_nodes); // reserve nodes pointers
  dof_id_type node_id = 0;

  // Add nodes to the new mesh
  for (unsigned int i = 0; i <= _peripheral_layer_num; ++i)
    for (unsigned int j = 0; j < input_ext_node_num; ++j)
    {
      nodes[node_id] = mesh->add_point(points_array[i][j], node_id);
      node_id_array[i][j] = node_id;
      node_id++;
    }
  // Add elements to the new mesh
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  for (unsigned int i = 0; i < _peripheral_layer_num; ++i)
    for (unsigned int j = 0; j < input_ext_node_num; ++j)
    {
      Elem * elem_Quad4 = mesh->add_elem(new Quad4);
      elem_Quad4->set_node(0) = nodes[node_id_array[i][j]];
      elem_Quad4->set_node(1) = nodes[node_id_array[i + 1][j]];
      elem_Quad4->set_node(2) = nodes[node_id_array[i + 1][(j + 1) % input_ext_node_num]];
      elem_Quad4->set_node(3) = nodes[node_id_array[i][(j + 1) % input_ext_node_num]];
      elem_Quad4->subdomain_id() = _peripheral_ring_block_id;

      if (i == 0)
        boundary_info.add_side(elem_Quad4, 3, OUTER_SIDESET_ID_ALT);
      if (i == _peripheral_layer_num - 1)
        boundary_info.add_side(elem_Quad4, 1, OUTER_SIDESET_ID);
    }

  // This would make sure that the boundary OUTER_SIDESET_ID is deleted after stitching.
  if (_input_mesh_external_bid != OUTER_SIDESET_ID)
    MooseMesh::changeBoundaryId(*input_mesh, _input_mesh_external_bid, OUTER_SIDESET_ID, true);
  mesh->prepare_for_use();
  mesh->stitch_meshes(*input_mesh, OUTER_SIDESET_ID_ALT, _input_mesh_external_bid, TOLERANCE, true);

  // Assign subdomain name to the new block if applicable
  if (isParamValid("peripheral_ring_block_name"))
    mesh->subdomain_name(_peripheral_ring_block_id) = _peripheral_ring_block_name;
  // Assign customized external boundary id
  if (_external_boundary_id > 0)
    MooseMesh::changeBoundaryId(*mesh, OUTER_SIDESET_ID, _external_boundary_id, false);
  // Assign customized external boundary name
  if (!_external_boundary_name.empty())
  {
    mesh->boundary_info->sideset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
    mesh->boundary_info->nodeset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}

bool
PeripheralRingMeshGenerator::isBoundaryValid(ReplicatedMesh & mesh,
                                             Real & max_node_radius,
                                             unsigned short & invalid_type,
                                             const Point origin_pt,
                                             const boundary_id_type bid) const
{
  max_node_radius = 0.0;
  invalid_type = 0;
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto side_list_tmp = boundary_info.build_side_list();
  unsigned int elem_counter = 0;
  std::vector<std::pair<dof_id_type, dof_id_type>> boundary_node_assm;
  std::vector<dof_id_type> boundary_ordered_node_list;
  bool isFlipped = false;
  for (unsigned int i = 0; i < side_list_tmp.size(); i++)
  {
    if (std::get<2>(side_list_tmp[i]) == bid)
    {
      elem_counter++;
      // store two nodes of each side
      boundary_node_assm.push_back(std::make_pair(mesh.elem_ptr(std::get<0>(side_list_tmp[i]))
                                                      ->side_ptr(std::get<1>(side_list_tmp[i]))
                                                      ->node_id(0),
                                                  mesh.elem_ptr(std::get<0>(side_list_tmp[i]))
                                                      ->side_ptr(std::get<1>(side_list_tmp[i]))
                                                      ->node_id(1)));
    }
  }
  // Start from the first element, try to find a chain of nodes
  boundary_ordered_node_list.push_back(boundary_node_assm.front().first);
  boundary_ordered_node_list.push_back(boundary_node_assm.front().second);
  // Remove the element that has been added to boundary_ordered_node_list
  boundary_node_assm.erase(boundary_node_assm.begin());
  const unsigned int boundary_node_assm_size_0 = boundary_node_assm.size();
  for (unsigned int i = 0; i < boundary_node_assm_size_0; i++)
  {
    // Find nodes to expand the chain
    dof_id_type end_node_id = boundary_ordered_node_list.back();
    auto isMatch1 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair) {
      return old_id_pair.first == end_node_id;
    };
    auto isMatch2 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair) {
      return old_id_pair.second == end_node_id;
    };
    auto result = std::find_if(boundary_node_assm.begin(), boundary_node_assm.end(), isMatch1);
    bool match_first;
    if (result == boundary_node_assm.end())
    {
      match_first = false;
      result = std::find_if(boundary_node_assm.begin(), boundary_node_assm.end(), isMatch2);
    }
    else
    {
      match_first = true;
    }
    // If found, add the node to boundary_ordered_node_list
    if (result != boundary_node_assm.end())
    {
      boundary_ordered_node_list.push_back(match_first ? (*result).second : (*result).first);
      boundary_node_assm.erase(result);
    }
    // If there are still elements in boundary_node_assm and result ==
    // boundary_node_assm.end(), this means the boundary is not a loop, the
    // boundary_ordered_node_list is flipped and try the other direction that has not
    // been examined yet.
    else
    {
      if (isFlipped)
      {
        // Flipped twice; this means the boundary has at least two segments.
        // This is invalid type #1
        invalid_type = 1;
        return false;
      }
      // mark the first flip event.
      isFlipped = true;
      std::reverse(boundary_ordered_node_list.begin(), boundary_ordered_node_list.end());
      // As this iteration is wasted, set the iterator backward
      i--;
    }
  }
  // If the code ever gets here, boundary_node_assm is empty.
  // If the isFlipped == true, the boundary is not a loop.
  // This is not done inside the loop just for some potential applications in the future.
  if (isFlipped)
  {
    // This is invalid type #2
    invalid_type = 2;
    return false;
  }
  // It the boundary is a loop, check if azimuthal angles change monotonically
  else
  {
    // Utilize cross product here.
    // If azimuthal angles change monotonically,
    // the z components of the cross products are always negative or positive.
    std::vector<Real> ordered_node_azi_list;
    for (unsigned int i = 0; i < boundary_ordered_node_list.size() - 1; i++)
    {
      ordered_node_azi_list.push_back(
          (*mesh.node_ptr(boundary_ordered_node_list[i]) - origin_pt)
              .cross(*mesh.node_ptr(boundary_ordered_node_list[i + 1]) - origin_pt)(2));
      // Use this opportunity to calculate maximum radius
      max_node_radius = std::max((*mesh.node_ptr(boundary_ordered_node_list[i]) - origin_pt).norm(),
                                 max_node_radius);
    }
    std::sort(ordered_node_azi_list.begin(), ordered_node_azi_list.end());
    if (ordered_node_azi_list.front() * ordered_node_azi_list.back() < 0.0)
    {
      // This is invalid type #3
      invalid_type = 3;
      return false;
    }
    else
      return true;
  }
}

bool
PeripheralRingMeshGenerator::isBoundaryValid(ReplicatedMesh & mesh,
                                             const Point origin_pt,
                                             const boundary_id_type bid) const
{
  Real dummy_max_node_radius;
  unsigned short dummy_invalid_type;
  return isBoundaryValid(mesh, dummy_max_node_radius, dummy_invalid_type, origin_pt, bid);
}

bool
PeripheralRingMeshGenerator::isExternalBoundary(ReplicatedMesh & mesh,
                                                const boundary_id_type bid) const
{
  if (!mesh.is_prepared())
    mesh.find_neighbors();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto side_list = boundary_info.build_side_list();
  for (unsigned int i = 0; i < side_list.size(); i++)
  {
    if (std::get<2>(side_list[i]) == bid)
      if (mesh.elem_ptr(std::get<0>(side_list[i]))->neighbor_ptr(std::get<1>(side_list[i])) !=
          nullptr)
        return false;
  }
  return true;
}
