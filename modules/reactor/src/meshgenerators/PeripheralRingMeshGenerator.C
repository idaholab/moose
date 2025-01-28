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
#include "MooseUtils.h"
#include "LinearInterpolation.h"
#include "FillBetweenPointVectorsTools.h"
#include "PolygonalMeshGenerationUtils.h"
#include "libmesh/int_range.h"

// C++ includes
#include <cmath> // for atan2

registerMooseObject("ReactorApp", PeripheralRingMeshGenerator);

InputParameters
PeripheralRingMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to be modified.");
  params.addParam<bool>(
      "force_input_centroid_as_center",
      false,
      "Whether to enforce use of the centroid position of the input mesh as the "
      "center of the peripheral ring by translating the input mesh to the origin.");

  params.addRangeCheckedParam<unsigned int>(
      "peripheral_layer_num",
      3,
      "peripheral_layer_num>0",
      "The radial layers of the peripheral ring to be added.");
  params.addRangeCheckedParam<Real>(
      "peripheral_radial_bias",
      1.0,
      "peripheral_radial_bias>0",
      "Value used to create biasing in radial meshing for peripheral ring region.");
  params.addRangeCheckedParam<Real>(
      "peripheral_inner_boundary_layer_width",
      0.0,
      "peripheral_inner_boundary_layer_width>=0",
      "Width of peripheral ring region that is assigned to be the inner boundary layer.");
  params.addRangeCheckedParam<unsigned int>(
      "peripheral_inner_boundary_layer_intervals",
      1,
      "peripheral_inner_boundary_layer_intervals>0",
      "Number of radial intervals of the peripheral ring inner boundary layer");
  params.addRangeCheckedParam<Real>(
      "peripheral_inner_boundary_layer_bias",
      1.0,
      "peripheral_inner_boundary_layer_bias>0",
      "Growth factor used for mesh biasing of the peripheral ring inner boundary layer.");
  params.addRangeCheckedParam<Real>(
      "peripheral_outer_boundary_layer_width",
      0.0,
      "peripheral_outer_boundary_layer_width>=0",
      "Width of peripheral ring region that is assigned to be the outer boundary layer.");
  params.addRangeCheckedParam<unsigned int>(
      "peripheral_outer_boundary_layer_intervals",
      1,
      "peripheral_outer_boundary_layer_intervals>0",
      "Number of radial intervals of the peripheral ring outer boundary layer");
  params.addRangeCheckedParam<Real>(
      "peripheral_outer_boundary_layer_bias",
      1.0,
      "peripheral_outer_boundary_layer_bias>0",
      "Growth factor used for mesh biasing of the peripheral ring outer boundary layer.");
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
  params.addParam<BoundaryName>(
      "external_boundary_name", "", "Optional customized external boundary name.");
  params.addParamNamesToGroup(
      "peripheral_radial_bias peripheral_inner_boundary_layer_width "
      "peripheral_inner_boundary_layer_intervals peripheral_inner_boundary_layer_bias "
      "peripheral_outer_boundary_layer_width peripheral_outer_boundary_layer_intervals "
      "peripheral_outer_boundary_layer_bias",
      "Mesh Boundary Layer and Biasing Options");
  params.addClassDescription("This PeripheralRingMeshGenerator object adds a circular peripheral "
                             "region to the input mesh.");

  return params;
}

PeripheralRingMeshGenerator::PeripheralRingMeshGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _force_input_centroid_as_center(getParam<bool>("force_input_centroid_as_center")),
    _peripheral_layer_num(getParam<unsigned int>("peripheral_layer_num")),
    _peripheral_radial_bias(getParam<Real>("peripheral_radial_bias")),
    _peripheral_inner_boundary_layer_params(
        {getParam<Real>("peripheral_inner_boundary_layer_width"),
         0.0,
         getParam<Real>("peripheral_inner_boundary_layer_width") > 0.0
             ? getParam<unsigned int>("peripheral_inner_boundary_layer_intervals")
             : 0,
         getParam<Real>("peripheral_inner_boundary_layer_bias")}),
    _peripheral_outer_boundary_layer_params(
        {getParam<Real>("peripheral_outer_boundary_layer_width"),
         0.0,
         getParam<Real>("peripheral_outer_boundary_layer_width") > 0.0
             ? getParam<unsigned int>("peripheral_outer_boundary_layer_intervals")
             : 0,
         getParam<Real>("peripheral_outer_boundary_layer_bias")}),
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
    _external_boundary_name(getParam<BoundaryName>("external_boundary_name")),
    _input(getMeshByName(_input_name))
{
  declareMeshProperty<bool>("hexagon_peripheral_trimmability", false);
  declareMeshProperty<bool>("hexagon_center_trimmability", false);
  declareMeshProperty<bool>("square_peripheral_trimmability", false);
  declareMeshProperty<bool>("square_center_trimmability", false);
}

std::unique_ptr<MeshBase>
PeripheralRingMeshGenerator::generate()
{
  if (hasMeshProperty<bool>("hexagon_center_trimmability", _input_name))
    setMeshProperty("hexagon_center_trimmability",
                    getMeshProperty<bool>("hexagon_center_trimmability", _input_name));
  if (hasMeshProperty<bool>("square_center_trimmability", _input_name))
    setMeshProperty("square_center_trimmability",
                    getMeshProperty<bool>("square_center_trimmability", _input_name));

  // Need ReplicatedMesh for stitching
  auto input_mesh = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!input_mesh)
    paramError("input", "Input is not a replicated mesh, which is required.");
  if (*(input_mesh->elem_dimensions().begin()) != 2 ||
      *(input_mesh->elem_dimensions().rbegin()) != 2)
    paramError("input", "Only 2D meshes are supported.");

  _input_mesh_external_bid =
      MooseMeshUtils::getBoundaryID(_input_mesh_external_boundary, *input_mesh);
  if (!MooseMeshUtils::hasBoundaryName(*input_mesh, _input_mesh_external_boundary))
    paramError("input_mesh_external_boundary",
               "External boundary does not exist in the input mesh");
  // We check the element types of input mesh's external boundary here.
  // We support both linear and quadratic sides (i.e., EDGE2 and EDGE3), but we cannot support mixed
  // sides
  auto side_list_tmp = input_mesh->get_boundary_info().build_side_list();
  bool linear_side = false;
  bool quadratic_side = false;
  for (const auto & side : side_list_tmp)
  {
    if (std::get<2>(side) == _input_mesh_external_bid)
    {
      const auto etype = input_mesh->elem_ptr(std::get<0>(side))->side_type(std::get<1>(side));
      if (etype == EDGE2)
        linear_side = true;
      else if (etype == EDGE3)
        quadratic_side = true;
      else
        paramError("input", "Input contains elements that are not supported by this generator.");
    }
  }
  if (linear_side && quadratic_side)
    paramError("input", "Input contains mixed element types on the external boundary.");
  const unsigned int order = linear_side ? 1 : 2;
  if (order == 2)
  {
    _peripheral_outer_boundary_layer_params.bias =
        std::sqrt(_peripheral_outer_boundary_layer_params.bias);
    _peripheral_inner_boundary_layer_params.bias =
        std::sqrt(_peripheral_inner_boundary_layer_params.bias);
    _peripheral_outer_boundary_layer_params.intervals *= 2;
    _peripheral_inner_boundary_layer_params.intervals *= 2;
    _peripheral_radial_bias = std::sqrt(_peripheral_radial_bias);
  }

  // Move the centroid of the mesh to (0, 0, 0) if input centroid is enforced to be the ring center.
  const Point input_mesh_centroid = MooseMeshUtils::meshCentroidCalculator(*input_mesh);
  if (_force_input_centroid_as_center)
    MeshTools::Modification::translate(
        *input_mesh, -input_mesh_centroid(0), -input_mesh_centroid(1), -input_mesh_centroid(2));

  // Use CoM of the input mesh as its origin for azimuthal calculation
  const Point origin_pt =
      _force_input_centroid_as_center ? Point(0.0, 0.0, 0.0) : input_mesh_centroid;
  // Vessel for containing maximum radius of the boundary nodes
  Real max_input_mesh_node_radius;

  // Calculate biasing terms
  // For 2nd order mesh, as we need the mid points, the bias factor is square rooted.
  const auto main_peripheral_bias_terms =
      biasTermsCalculator(_peripheral_radial_bias, _peripheral_layer_num * order);
  const auto inner_peripheral_bias_terms =
      biasTermsCalculator(_peripheral_inner_boundary_layer_params.bias,
                          _peripheral_inner_boundary_layer_params.intervals);
  // It is easier to create outer boundary layer inversely (inwards). Thus, 1.0 / bias is used here.
  // However, the input parameter definition is not affected.
  const auto outer_peripheral_bias_terms =
      biasTermsCalculator(1.0 / _peripheral_outer_boundary_layer_params.bias,
                          _peripheral_outer_boundary_layer_params.intervals);

  const unsigned int total_peripheral_layer_num =
      _peripheral_inner_boundary_layer_params.intervals + _peripheral_layer_num * order +
      _peripheral_outer_boundary_layer_params.intervals;

  // Collect the external boundary nodes of the input mesh using the utility function
  std::vector<dof_id_type> boundary_ordered_node_list;
  try
  {
    FillBetweenPointVectorsTools::isBoundarySimpleClosedLoop(*input_mesh,
                                                             max_input_mesh_node_radius,
                                                             boundary_ordered_node_list,
                                                             origin_pt,
                                                             _input_mesh_external_bid);
  }
  catch (MooseException & e)
  {
    if (((std::string)e.what()).compare("The node list provided has more than one segments.") == 0)
      paramError("input_mesh_external_boundary",
                 "This mesh generator does not work for the provided external boundary as it has "
                 "more than one segments.");
    else
      paramError("input_mesh_external_boundary", e.what());
  }

  if (max_input_mesh_node_radius >= _peripheral_ring_radius)
    paramError(
        "peripheral_ring_radius",
        "The peripheral ring to be generated must be large enough to cover the entire input mesh.");
  if (!FillBetweenPointVectorsTools::isExternalBoundary(*input_mesh, _input_mesh_external_bid))
    paramError("input_mesh_external_boundary",
               "The boundary provided is not an external boundary.");

  std::vector<Point> input_ext_bd_pts;
  std::vector<Point> output_ext_bd_pts;
  std::vector<std::tuple<Real, Point, Point>> azi_points;
  std::vector<Real> azi_array;
  Real tmp_azi(0.0);
  auto mesh = buildReplicatedMesh(2);

  // Node counter for the external boundary
  unsigned int input_ext_node_num = 0;

  // As the node list collected before is a simple closed loop, the last node is the same as the
  // first node. We remove it here.
  boundary_ordered_node_list.pop_back();
  // Loop over all the boundary nodes
  // For quadratic sides, the middle points are included automatically
  for (const auto i : index_range(boundary_ordered_node_list))
  {
    input_ext_node_num++;
    // Define nodes on the inner and outer boundaries of the peripheral region.
    input_ext_bd_pts.push_back(input_mesh->point(boundary_ordered_node_list[i]));
    tmp_azi =
        atan2(input_ext_bd_pts.back()(1) - origin_pt(1), input_ext_bd_pts.back()(0) - origin_pt(0));
    output_ext_bd_pts.push_back(Point(_peripheral_ring_radius * std::cos(tmp_azi),
                                      _peripheral_ring_radius * std::sin(tmp_azi),
                                      origin_pt(2)));
    // a vector of tuples using azimuthal angle as the key to facilitate sorting
    azi_points.push_back(
        std::make_tuple(tmp_azi, input_ext_bd_pts.back(), output_ext_bd_pts.back()));
    // a simple vector of azimuthal angles for radius correction purposes (in degree)
    azi_array.push_back(tmp_azi / M_PI * 180.0);
  }
  // Only for quadratic sides, if the index of the first node after sorting is even, it is a vertex
  // node; otherwise, it is a midpoint node.
  const bool is_first_node_vertex =
      order == 1 ||
      !(std::distance(azi_array.begin(), std::min_element(azi_array.begin(), azi_array.end())) % 2);
  std::sort(azi_points.begin(), azi_points.end());
  std::sort(azi_array.begin(), azi_array.end());

  // Angles defined by three neighboring nodes on input mesh's external boundary
  std::vector<Real> input_bdry_angles;
  // Normal directions of input boundary nodes
  std::vector<Point> input_bdry_nd;
  for (const auto i : index_range(azi_points))
  {
    Point p1, p2;
    const Point pn = Point(0.0, 0.0, 1.0);
    if (i == 0)
    {
      p1 = std::get<1>(azi_points[i + 1]) - std::get<1>(azi_points[i]);
      p2 = std::get<1>(azi_points.back()) - std::get<1>(azi_points[i]);
    }
    else if (i == azi_points.size() - 1)
    {
      p1 = std::get<1>(azi_points.front()) - std::get<1>(azi_points.back());
      p2 = std::get<1>(azi_points[i - 1]) - std::get<1>(azi_points.back());
    }
    else
    {
      p1 = std::get<1>(azi_points[i + 1]) - std::get<1>(azi_points[i]);
      p2 = std::get<1>(azi_points[i - 1]) - std::get<1>(azi_points[i]);
    }
    // Use cross point to get perpendicular direction
    const Point p1n = (p1.cross(pn)).unit();
    const Point p2n = -(p2.cross(pn)).unit();
    Real tmp = p1 * p2 / p1.norm() / p2.norm();
    // Make sure acos() gets valid input
    tmp = tmp > 1.0 ? 1.0 : (tmp < -1.0 ? -1.0 : tmp);
    input_bdry_angles.push_back(acos(tmp) / 2.0);
    input_bdry_nd.push_back((p1n + p2n).unit());
  }

  // 2D vector containing all the node positions of the peripheral region
  std::vector<std::vector<Point>> points_array(total_peripheral_layer_num + 1,
                                               std::vector<Point>(input_ext_node_num));
  // 2D vector containing all the node ids of the peripheral region
  std::vector<std::vector<dof_id_type>> node_id_array(total_peripheral_layer_num + 1,
                                                      std::vector<dof_id_type>(input_ext_node_num));
  // Reference outmost layer of inner boundary layer
  std::vector<Point> ref_inner_bdry_surf;
  // First loop
  for (const auto i : make_range(input_ext_node_num))
  {
    // Inner boundary nodes of the peripheral region
    points_array[0][i] = std::get<1>(azi_points[i]);
    // Define outer layer of inner boundary layer
    if (_peripheral_inner_boundary_layer_params.intervals)
    {
      // Outside point of the inner boundary layer
      const Point ref_inner_boundary_shift =
          (_peripheral_inner_boundary_layer_params.width / sin(input_bdry_angles[i])) *
          input_bdry_nd[i];
      ref_inner_bdry_surf.push_back(points_array[0][i] + ref_inner_boundary_shift);
    }
  }

  if (_peripheral_inner_boundary_layer_params.intervals)
    innerBdryLayerNodesDefiner(input_ext_node_num,
                               input_bdry_angles,
                               ref_inner_bdry_surf,
                               inner_peripheral_bias_terms,
                               azi_array,
                               origin_pt,
                               points_array);
  const Real correction_factor = _preserve_volumes
                                     ? PolygonalMeshGenerationUtils::radiusCorrectionFactor(
                                           azi_array, true, order, is_first_node_vertex)
                                     : 1.0;
  // Loop to handle outer boundary layer and main region
  for (const auto i : make_range(input_ext_node_num))
  {
    // Outer boundary nodes of the peripheral region
    points_array[total_peripheral_layer_num][i] = std::get<2>(azi_points[i]) * correction_factor;
    // Outer boundary layer points
    if (_peripheral_outer_boundary_layer_params.intervals)
    {
      // Inner point of the outer boundary layer
      const Point outer_boundary_shift =
          -Point(std::cos(std::get<0>(azi_points[i])), std::sin(std::get<0>(azi_points[i])), 0.0) *
          _peripheral_outer_boundary_layer_params.width;
      for (const auto j :
           make_range(uint(1), _peripheral_outer_boundary_layer_params.intervals + 1))
        points_array[total_peripheral_layer_num - j][i] =
            points_array[total_peripheral_layer_num][i] +
            outer_boundary_shift * outer_peripheral_bias_terms[j - 1];
    }
    // Use interpolation to get main region
    if (MooseUtils::absoluteFuzzyGreaterEqual(
            (points_array[_peripheral_inner_boundary_layer_params.intervals][i] - origin_pt).norm(),
            (points_array[_peripheral_inner_boundary_layer_params.intervals +
                          _peripheral_layer_num * order][i] -
             origin_pt)
                .norm()))
    {
      paramError("peripheral_inner_boundary_layer_width",
                 "The summation of peripheral_inner_boundary_layer_width and "
                 "peripheral_outer_boundary_layer_width must be smaller than the thickness of "
                 "peripheral ring region.");
    }

    for (const auto j : make_range(uint(1), _peripheral_layer_num * order))
      points_array[j + _peripheral_inner_boundary_layer_params.intervals][i] =
          points_array[_peripheral_inner_boundary_layer_params.intervals][i] *
              (1.0 - main_peripheral_bias_terms[j - 1]) +
          points_array[_peripheral_inner_boundary_layer_params.intervals +
                       _peripheral_layer_num * order][i] *
              main_peripheral_bias_terms[j - 1];
  }
  unsigned int num_total_nodes = (total_peripheral_layer_num + 1) * input_ext_node_num;
  std::vector<Node *> nodes(num_total_nodes); // reserve nodes pointers
  dof_id_type node_id = 0;

  // Add nodes to the new mesh
  for (const auto i : make_range(total_peripheral_layer_num + 1))
    for (const auto j : make_range(input_ext_node_num))
    {
      nodes[node_id] = mesh->add_point(points_array[i][j], node_id);
      node_id_array[i][j] = node_id;
      node_id++;
    }
  // Add elements to the new mesh
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  const unsigned int index_shift = is_first_node_vertex ? 0 : 1;
  for (const auto i : make_range(total_peripheral_layer_num / order))
    for (const auto j : make_range(input_ext_node_num / order))
    {
      std::unique_ptr<Elem> new_elem;
      new_elem = std::make_unique<Quad4>();
      if (order == 2)
      {
        new_elem = std::make_unique<Quad9>();
        new_elem->set_node(4) = nodes[node_id_array[i * order + 1][j * order + index_shift]];
        new_elem->set_node(5) = nodes[node_id_array[(i + 1) * order][(j * order + 1 + index_shift) %
                                                                     input_ext_node_num]];
        new_elem->set_node(6) = nodes[node_id_array[i * order + 1][((j + 1) * order + index_shift) %
                                                                   input_ext_node_num]];
        new_elem->set_node(7) =
            nodes[node_id_array[i * order][(j * order + 1 + index_shift) % input_ext_node_num]];
        new_elem->set_node(8) =
            nodes[node_id_array[i * order + 1][(j * order + 1 + index_shift) % input_ext_node_num]];
      }
      new_elem->set_node(0) = nodes[node_id_array[i * order][j * order + index_shift]];
      new_elem->set_node(1) = nodes[node_id_array[(i + 1) * order][j * order + index_shift]];
      new_elem->set_node(2) = nodes[node_id_array[(i + 1) * order][((j + 1) * order + index_shift) %
                                                                   input_ext_node_num]];
      new_elem->set_node(3) =
          nodes[node_id_array[i * order][((j + 1) * order + index_shift) % input_ext_node_num]];
      new_elem->subdomain_id() = _peripheral_ring_block_id;

      Elem * added_elem = mesh->add_elem(std::move(new_elem));

      if (i == 0)
        boundary_info.add_side(added_elem, 3, OUTER_SIDESET_ID_ALT);
      if (i == total_peripheral_layer_num / order - 1)
        boundary_info.add_side(added_elem, 1, OUTER_SIDESET_ID);
    }

  // This would make sure that the boundary OUTER_SIDESET_ID is deleted after stitching.
  if (_input_mesh_external_bid != OUTER_SIDESET_ID)
    MooseMesh::changeBoundaryId(*input_mesh, _input_mesh_external_bid, OUTER_SIDESET_ID, false);
  mesh->prepare_for_use();
  // Use input_mesh here to retain the subdomain name map
  input_mesh->stitch_meshes(*mesh, OUTER_SIDESET_ID, OUTER_SIDESET_ID_ALT, TOLERANCE, true);

  // Assign subdomain name to the new block if applicable
  if (isParamValid("peripheral_ring_block_name"))
    input_mesh->subdomain_name(_peripheral_ring_block_id) = _peripheral_ring_block_name;
  // Assign customized external boundary id
  if (_external_boundary_id > 0)
    MooseMesh::changeBoundaryId(*input_mesh, OUTER_SIDESET_ID, _external_boundary_id, false);
  // Assign customized external boundary name
  if (!_external_boundary_name.empty())
  {
    input_mesh->get_boundary_info().sideset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
    input_mesh->get_boundary_info().nodeset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
  }

  _input->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(_input);
}

void
PeripheralRingMeshGenerator::innerBdryLayerNodesDefiner(
    const unsigned int input_ext_node_num,
    const std::vector<Real> input_bdry_angles,
    const std::vector<Point> ref_inner_bdry_surf,
    const std::vector<Real> inner_peripheral_bias_terms,
    const std::vector<Real> azi_array,
    const Point origin_pt,
    std::vector<std::vector<Point>> & points_array) const
{
  // Check if any azimuthal angles are messed after inner boundary layer addition
  std::vector<bool> delete_mark(input_ext_node_num, false);
  std::vector<Real> interp_azi_data, interp_x_data, interp_y_data;
  std::unique_ptr<LinearInterpolation> linterp_x, linterp_y;
  // Mark the to-be-deleted elements
  for (const auto i : make_range(input_ext_node_num))
  {
    // For a zig-zag external boundary, when we add a conformal layer onto it by translating the
    // nodes in the surface normal direction, it is possible that the azimuthal order is flipped.
    // As shown below, o's are the original boundary nodes, and *'s are the nodes after
    // translation by the boundary layer thickness.
    //                         *  *
    // |               |       | /
    // o               o-------/--*
    // |               |     / |
    // | outside  -->  |   /   |
    // |               | /     |
    // o--------o--    o-------o--
    // To mitigate this flipping issue, we check the node flipping here using the cross product of
    // neighboring node-to-origin vectors. Flipped nodes are marked and excluded during the
    // follow-up interpolation.
    if (!MooseUtils::absoluteFuzzyEqual(input_bdry_angles[i], M_PI / 2.0, 1.0e-4))
    {
      unsigned int index_shift = 1;
      bool minus_shift_flag = true;
      bool plus_shift_flag = true;
      // Need to check both directions for multiple shifts
      // Half of the external boundary nodes are used as an upper limit to avert infinite loops
      while (index_shift < input_ext_node_num / 2 && (minus_shift_flag || plus_shift_flag))
      {
        if (((ref_inner_bdry_surf[MathUtils::euclideanMod(((int)i - (int)index_shift),
                                                          (int)input_ext_node_num)] -
              origin_pt)
                 .cross(ref_inner_bdry_surf[i] - origin_pt))(2) <= 0.0 &&
            minus_shift_flag)
          delete_mark[MathUtils::euclideanMod(((int)i - (int)index_shift),
                                              (int)input_ext_node_num)] = true;
        else
          minus_shift_flag = false;
        if (((ref_inner_bdry_surf[(i + index_shift) % input_ext_node_num] - origin_pt)
                 .cross(ref_inner_bdry_surf[i] - origin_pt))(2) >= 0.0 &&
            plus_shift_flag)
          delete_mark[(i + index_shift) % input_ext_node_num] = true;
        else
          plus_shift_flag = false;
        index_shift++;
      }
    }
  }
  // Create vectors for interpolation
  // Due to the flip issue, linear interpolation is used here to mark the location of the boundary
  // layer's outer boundary.
  for (const auto i : make_range(input_ext_node_num))
  {
    if (!delete_mark[i])
    {
      interp_azi_data.push_back(atan2(ref_inner_bdry_surf[i](1) - origin_pt(1),
                                      ref_inner_bdry_surf[i](0) - origin_pt(0)));
      interp_x_data.push_back(ref_inner_bdry_surf[i](0));
      interp_y_data.push_back(ref_inner_bdry_surf[i](1));
      if (interp_azi_data.size() > 1)
        if (interp_azi_data.back() < interp_azi_data[interp_azi_data.size() - 2])
          interp_azi_data.back() += 2.0 * M_PI;
    }
  }
  const Real interp_x0 = interp_x_data.front();
  const Real interp_xt = interp_x_data.back();
  const Real interp_y0 = interp_y_data.front();
  const Real interp_yt = interp_y_data.back();
  const Real incept_azi0 = interp_azi_data.front();
  const Real incept_azit = interp_azi_data.back();

  if (MooseUtils::absoluteFuzzyGreaterThan(incept_azi0, -M_PI))
  {
    interp_azi_data.insert(interp_azi_data.begin(), incept_azit - 2.0 * M_PI);
    interp_x_data.insert(interp_x_data.begin(), interp_xt);
    interp_y_data.insert(interp_y_data.begin(), interp_yt);
  }
  else
    interp_azi_data.front() = -M_PI;
  if (MooseUtils::absoluteFuzzyLessThan(incept_azit, M_PI))
  {
    interp_azi_data.push_back(incept_azi0 + 2 * M_PI);
    interp_x_data.push_back(interp_x0);
    interp_y_data.push_back(interp_y0);
  }
  else
    interp_azi_data.back() = M_PI;

  // Establish interpolation
  linterp_x = std::make_unique<LinearInterpolation>(interp_azi_data, interp_x_data);
  linterp_y = std::make_unique<LinearInterpolation>(interp_azi_data, interp_y_data);
  //  Loop to handle inner boundary layer
  for (const auto i : make_range(input_ext_node_num))
  {
    // Outside point of the inner boundary layer
    // Using interpolation, the azimuthal angles do not need to be changed.
    const Point inner_boundary_shift = Point(linterp_x->sample(azi_array[i] / 180.0 * M_PI),
                                             linterp_y->sample(azi_array[i] / 180.0 * M_PI),
                                             origin_pt(2)) -
                                       points_array[0][i];
    for (const auto j : make_range(uint(1), _peripheral_inner_boundary_layer_params.intervals + 1))
      points_array[j][i] =
          points_array[0][i] + inner_boundary_shift * inner_peripheral_bias_terms[j - 1];
  }
}
