//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CircularBoundaryCorrectionGenerator.h"

#include "MooseUtils.h"
#include "LinearInterpolation.h"
#include "FillBetweenPointVectorsTools.h"
#include "CastUniquePointer.h"

// C++ includes
#include <cmath> // for atan2

registerMooseObject("MooseApp", CircularBoundaryCorrectionGenerator);

InputParameters
CircularBoundaryCorrectionGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to be modified.");

  params.addParam<bool>("move_end_nodes_in_span_direction",
                        false,
                        "Whether to move the end nodes in the span direction of the arc or not.");

  params.addRequiredParam<std::vector<BoundaryName>>("input_mesh_circular_boundaries",
                                                     "Names of the circular boundaries.");

  params.addRangeCheckedParam<std::vector<Real>>(
      "transition_layer_ratios",
      "transition_layer_ratios>0&transition_layer_ratios<=1.0",
      "Ratios to radii as the transition layers on both sides of the original radii (if this "
      "parameter is not provided, 0.01 will be used).");

  params.addRangeCheckedParam<std::vector<Real>>(
      "custom_circular_tolerance",
      "custom_circular_tolerance>0",
      "Custom tolerances (L2 norm) used to verify whether the provided boundaries are circular "
      "(default is 1.0e-6).");

  params.addClassDescription(
      "This CircularBoundaryCorrectionGenerator object is designed to correct full "
      "or partial circular boundaries in a 2D mesh to preserve areas.");

  return params;
}

CircularBoundaryCorrectionGenerator::CircularBoundaryCorrectionGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _input_mesh_circular_boundaries(
        getParam<std::vector<BoundaryName>>("input_mesh_circular_boundaries")),
    _transition_layer_ratios(isParamValid("transition_layer_ratios")
                                 ? getParam<std::vector<Real>>("transition_layer_ratios")
                                 : std::vector<Real>(_input_mesh_circular_boundaries.size(), 0.01)),
    _custom_circular_tolerance(
        isParamValid("custom_circular_tolerance")
            ? (getParam<std::vector<Real>>("custom_circular_tolerance").size() == 1
                   ? std::vector<Real>(
                         _input_mesh_circular_boundaries.size(),
                         getParam<std::vector<Real>>("custom_circular_tolerance").front())
                   : getParam<std::vector<Real>>("custom_circular_tolerance"))
            : std::vector<Real>(_input_mesh_circular_boundaries.size(), 1.0e-6)),
    _move_end_nodes_in_span_direction(getParam<bool>("move_end_nodes_in_span_direction")),
    _input(getMeshByName(_input_name))
{
  if (_transition_layer_ratios.size() != _input_mesh_circular_boundaries.size())
    paramError("transition_layer_ratios",
               "this parameter must have the same length as 'input_mesh_circular_boundaries'.");
  if (_custom_circular_tolerance.size() != _input_mesh_circular_boundaries.size())
    paramError("custom_circular_tolerance",
               "if provided, this parameter must have either a unity length or the same length as "
               "'input_mesh_circular_boundaries'.");
}

std::unique_ptr<MeshBase>
CircularBoundaryCorrectionGenerator::generate()
{
  auto input_mesh = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!input_mesh)
    paramError("input", "Input is not a replicated mesh, which is required");

  // Check if the input mesh has the given boundaries
  for (const auto & bd : _input_mesh_circular_boundaries)
  {
    if (!MooseMeshUtils::hasBoundaryName(*input_mesh, bd))
      paramError("input_mesh_circular_boundaries",
                 "the boundary '" + bd + "' does not exist in the input mesh.");
  }
  _input_mesh_circular_bids =
      MooseMeshUtils::getBoundaryIDs(*input_mesh, _input_mesh_circular_boundaries, false);

  auto side_list = input_mesh->get_boundary_info().build_side_list();
  input_mesh->get_boundary_info().build_node_list_from_side_list();
  auto node_list = input_mesh->get_boundary_info().build_node_list();

  std::vector<std::vector<Point>> input_circ_bds_pts(_input_mesh_circular_bids.size());
  // Loop over all the boundary nodes and store the nodes (points) on each of the circular
  // boundaries to be corrected
  for (unsigned int i = 0; i < node_list.size(); ++i)
  {
    auto node_list_it = std::find(_input_mesh_circular_bids.begin(),
                                  _input_mesh_circular_bids.end(),
                                  std::get<1>(node_list[i]));
    if (node_list_it != _input_mesh_circular_bids.end())
    {
      input_circ_bds_pts[std::distance(_input_mesh_circular_bids.begin(), node_list_it)].push_back(
          input_mesh->point(std::get<0>(node_list[i])));
      if (!MooseUtils::absoluteFuzzyEqual(
              input_circ_bds_pts[std::distance(_input_mesh_circular_bids.begin(), node_list_it)]
                  .back()(2),
              0.0))
        paramError("input_mesh_circular_boundaries", "Only boundaries in XY plane are supported.");
    }
  }
  std::vector<std::vector<std::pair<Point, Point>>> input_circ_bds_sds(
      _input_mesh_circular_bids.size());
  std::vector<std::vector<std::pair<dof_id_type, dof_id_type>>> input_circ_bds_sds_ids(
      _input_mesh_circular_bids.size());
  // Loop over all the boundary sides and store the sides (node pairs) on each of the circular
  // boundaries to be corrected
  for (unsigned int i = 0; i < side_list.size(); ++i)
  {
    auto side_list_it = std::find(_input_mesh_circular_bids.begin(),
                                  _input_mesh_circular_bids.end(),
                                  std::get<2>(side_list[i]));
    if (side_list_it != _input_mesh_circular_bids.end())
    {
      const auto side =
          input_mesh->elem_ptr(std::get<0>(side_list[i]))->side_ptr(std::get<1>(side_list[i]));
      // In case the sideset contains a pair of duplicated sides with different orientations, we
      // only store the first side found.
      if (std::find(
              input_circ_bds_sds_ids[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
                  .begin(),
              input_circ_bds_sds_ids[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
                  .end(),
              std::make_pair(side->node_id(0), side->node_id(1))) ==
              input_circ_bds_sds_ids[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
                  .end() &&
          std::find(
              input_circ_bds_sds_ids[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
                  .begin(),
              input_circ_bds_sds_ids[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
                  .end(),
              std::make_pair(side->node_id(1), side->node_id(0))) ==
              input_circ_bds_sds_ids[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
                  .end())
      {
        input_circ_bds_sds[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
            .push_back(std::make_pair(side->point(0), side->point(1)));
        input_circ_bds_sds_ids[std::distance(_input_mesh_circular_bids.begin(), side_list_it)]
            .push_back(std::make_pair(side->node_id(0), side->node_id(1)));
      }
    }
  }

  // Computes the correction coefficient and apply it;
  // Also records the nodes that have been modified to avert double-correction
  std::set<dof_id_type> mod_node_list;
  // And check if any of the boundaries are partial circles
  bool has_partial_circle = false;
  for (unsigned int i = 0; i < input_circ_bds_pts.size(); i++)
  {
    auto & input_circ_bd_pts = input_circ_bds_pts[i];
    if (input_circ_bd_pts.size() < 3)
      paramError("input_mesh_circular_boundaries",
                 "too few nodes in one of the provided boundaries.");
    Real bdry_rad;
    const Point boundary_origin =
        circularCenterCalculator(input_circ_bd_pts, bdry_rad, _custom_circular_tolerance[i]);

    // Check if the boundary is a full circle or partial
    // Also make an ordered array of nodes
    Real dummy_max_rad;
    std::vector<dof_id_type> ordered_node_list;
    bool is_bdry_closed;
    FillBetweenPointVectorsTools::isClosedLoop(*input_mesh,
                                               dummy_max_rad,
                                               ordered_node_list,
                                               input_circ_bds_sds_ids[i],
                                               boundary_origin,
                                               "boundary",
                                               is_bdry_closed,
                                               true);

    has_partial_circle = has_partial_circle || !is_bdry_closed;
    // If the user selects to move the end nodes of a partial circular boundary, we need to
    // calculate the displacement of the end nodes, which differs from the displacement of the
    // other nodes
    Real end_node_disp;
    // c_coeff is the coefficient used to scale the azimuthal angles
    // corr_factor > 1 means the partial boundary is less than a half circle
    // corr_factor < 1 means the partial boundary is more than a half circle
    // corr_factor = 1 means the boundary is a full circle or a half circle
    Real c_coeff;
    const Real corr_factor = generateRadialCorrectionFactor(input_circ_bds_sds[i],
                                                            boundary_origin,
                                                            is_bdry_closed,
                                                            _move_end_nodes_in_span_direction,
                                                            c_coeff,
                                                            end_node_disp);
    // Radial range that within which the nodes will be moved
    const Real transition_layer_thickness = _transition_layer_ratios[i] * bdry_rad;
    // For a partial boundary, we take out the start and end nodes from the ordered node list
    const Point pt_start = input_mesh->point(ordered_node_list.front());
    const Point pt_end = input_mesh->point(ordered_node_list.back());
    // The direction of the span of the partial boundary (which is an arc)
    const Point span_direction = is_bdry_closed ? Point(0.0, 0.0, 0.0) : (pt_start - pt_end).unit();
    // Although these are also calculated for a full circular boundary, they are not used

    // Find the center of the circular boundary's azimuthal angle and the half of the azimuthal
    // range
    Real center_bdry_azi;
    Real half_azi_range;
    if (MooseUtils::absoluteFuzzyEqual(c_coeff, 1.0))
    {
      // For a half circle, the center boundary azimuthal angle is the starting node's azimuthal
      // plus or minus pi/2
      const Point pt_second = input_mesh->point(ordered_node_list[1]);
      const Real pt_start_azi =
          std::atan2(pt_start(1) - boundary_origin(1), pt_start(0) - boundary_origin(0));
      const Real pt_second_azi =
          std::atan2(pt_second(1) - boundary_origin(1), pt_second(0) - boundary_origin(0));
      // Figure out whether it is plus or minus pi/2
      const Real azi_direction =
          (pt_second_azi - pt_start_azi > 0 or pt_second_azi - pt_start_azi < -M_PI) ? 1.0 : -1.0;
      center_bdry_azi = pt_start_azi + azi_direction * M_PI / 2.0;
      half_azi_range = M_PI / 2.0;
    }
    else
    {
      const Point center_bdry =
          (c_coeff > 1.0 ? 1.0 : -1.0) * (pt_start + pt_end) / 2.0 - boundary_origin;
      const Real pt_start_azi =
          std::atan2(pt_start(1) - boundary_origin(1), pt_start(0) - boundary_origin(0));
      center_bdry_azi = std::atan2(center_bdry(1), center_bdry(0));
      half_azi_range = std::abs(pt_start_azi - center_bdry_azi > M_PI
                                    ? pt_start_azi - center_bdry_azi - 2 * M_PI
                                    : (pt_start_azi - center_bdry_azi < -M_PI
                                           ? pt_start_azi - center_bdry_azi + 2 * M_PI
                                           : pt_start_azi - center_bdry_azi));
    }
    // Loop over all the nodes in the mesh to move the nodes in the transition layer so that the
    // circular boundary is corrected
    for (auto & node : input_mesh->node_ptr_range())
    {
      // If the end nodes are set to move in the span direction, we should do so
      if (node->id() == ordered_node_list.front() && _move_end_nodes_in_span_direction &&
          end_node_disp >= 0.0)
      {
        if (!mod_node_list.emplace((*node).id()).second)
          paramError("transition_layer_ratios", "the transition layers are overlapped.");
        (*node) = (*node) + end_node_disp * bdry_rad * span_direction;
        continue;
      }
      if (node->id() == ordered_node_list.back() && _move_end_nodes_in_span_direction &&
          end_node_disp >= 0.0)
      {
        if (!mod_node_list.emplace((*node).id()).second)
          paramError("transition_layer_ratios", "the transition layers are overlapped.");
        (*node) = (*node) - end_node_disp * bdry_rad * span_direction;
        continue;
      }
      const Point tmp_pt = (*node) - boundary_origin;
      const Real tmp_pt_azi = std::atan2(tmp_pt(1), tmp_pt(0));
      const Real angle_diff =
          tmp_pt_azi - center_bdry_azi > M_PI
              ? tmp_pt_azi - center_bdry_azi - 2.0 * M_PI
              : (tmp_pt_azi - center_bdry_azi < -M_PI ? tmp_pt_azi - center_bdry_azi + 2.0 * M_PI
                                                      : tmp_pt_azi - center_bdry_azi);
      const Real pt_rad = tmp_pt.norm();
      // Skip the node if it is not within the radial and azimuthal range
      if ((!is_bdry_closed && MooseUtils::absoluteFuzzyGreaterThan(
                                  std::abs(angle_diff), half_azi_range, libMesh::TOLERANCE)) ||
          pt_rad < bdry_rad - transition_layer_thickness ||
          pt_rad > bdry_rad + transition_layer_thickness)
        continue;
      const Real tmp_pt_azi_new =
          (_move_end_nodes_in_span_direction ? c_coeff : 1.0) * angle_diff + center_bdry_azi;
      const Real mod_corr_factor =
          pt_rad <= bdry_rad
              ? (1.0 + (corr_factor - 1.0) * (pt_rad - bdry_rad + transition_layer_thickness) /
                           transition_layer_thickness)
              : (1.0 + (corr_factor - 1.0) * (bdry_rad + transition_layer_thickness - pt_rad) /
                           transition_layer_thickness);
      if (!MooseUtils::absoluteFuzzyEqual(mod_corr_factor, 1.0))
      {
        if (!mod_node_list.emplace((*node).id()).second)
          paramError("transition_layer_ratios", "the transition layers are overlapped.");
      }
      const Point tmp_pt_alt =
          Point(pt_rad * std::cos(tmp_pt_azi_new), pt_rad * std::sin(tmp_pt_azi_new), tmp_pt(2)) *
          mod_corr_factor;
      (*node) = tmp_pt_alt + boundary_origin;
    }
  }
  if (!has_partial_circle && _move_end_nodes_in_span_direction)
    paramError("move_end_nodes_in_span_direction",
               "all the boundaries are closed, this parameter should be be set as 'true'.");
  // Loop over all the elements to check if any of them are inverted due to the correction
  for (auto & elem : input_mesh->element_ptr_range())
    if (elem->volume() < 0.0)
      paramError("transition_layer_ratios",
                 "some elements are inverted during circular boundary correction, consider tuning "
                 "this parameter.");

  return dynamic_pointer_cast<MeshBase>(_input);
}

Point
CircularBoundaryCorrectionGenerator::circularCenterCalculator(const std::vector<Point> & pts_list,
                                                              Real & radius,
                                                              const Real tol) const
{
  // Usually, just using the first three points would work
  // Here a more complex selection is made in case the first three points are too close to each
  // other.
  // Step 1: find the farthest point from the first point
  Real tmp_dis(0.0);
  unsigned int farthest_pt_id(0);
  for (unsigned int i = 1; i < pts_list.size(); i++)
    if ((pts_list[i] - pts_list[0]).norm() > tmp_dis)
    {
      tmp_dis = (pts_list[i] - pts_list[0]).norm();
      farthest_pt_id = i;
    }
  // Step 2: find the third point that is nearly in the middle of the first two
  unsigned int mid_pt_id(0);
  for (unsigned int i = 1; i < pts_list.size(); i++)
    if (i != farthest_pt_id && std::abs((pts_list[farthest_pt_id] - pts_list[i]).norm() -
                                        (pts_list[0] - pts_list[i]).norm()) < tmp_dis)
    {
      tmp_dis = std::abs((pts_list[farthest_pt_id] - pts_list[i]).norm() -
                         (pts_list[0] - pts_list[i]).norm());
      mid_pt_id = i;
    }

  const Real x12 = pts_list[0](0) - pts_list[farthest_pt_id](0);
  const Real x13 = pts_list[0](0) - pts_list[mid_pt_id](0);

  const Real y12 = pts_list[0](1) - pts_list[farthest_pt_id](1);
  const Real y13 = pts_list[0](1) - pts_list[mid_pt_id](1);

  const Real sx13 =
      pts_list[0](0) * pts_list[0](0) - pts_list[mid_pt_id](0) * pts_list[mid_pt_id](0);
  const Real sy13 =
      pts_list[0](1) * pts_list[0](1) - pts_list[mid_pt_id](1) * pts_list[mid_pt_id](1);

  const Real sx21 =
      pts_list[farthest_pt_id](0) * pts_list[farthest_pt_id](0) - pts_list[0](0) * pts_list[0](0);
  const Real sy21 =
      pts_list[farthest_pt_id](1) * pts_list[farthest_pt_id](1) - pts_list[0](1) * pts_list[0](1);

  const Real f =
      (sx13 * x12 + sy13 * x12 + sx21 * x13 + sy21 * x13) / (2.0 * (-y13 * x12 + y12 * x13));
  const Real g =
      (sx13 * y12 + sy13 * y12 + sx21 * y13 + sy21 * y13) / (2.0 * (-x13 * y12 + x12 * y13));

  const Real c = -pts_list[0](0) * pts_list[0](0) - pts_list[0](1) * pts_list[0](1) -
                 2.0 * g * pts_list[0](0) - 2.0 * f * pts_list[0](1);
  const Real r2 = f * f + g * g - c;

  radius = std::sqrt(r2);
  const Point circ_origin = Point(-g, -f, 0.0);

  for (const auto & pt : pts_list)
    if (!MooseUtils::absoluteFuzzyEqual((pt - circ_origin).norm(), radius, tol))
      paramError("input_mesh_circular_boundaries", "the boundary provided is not circular.");
  return circ_origin;
}

Real
CircularBoundaryCorrectionGenerator::generateRadialCorrectionFactor(
    const std::vector<std::pair<Point, Point>> & bd_side_list,
    const Point & circle_center,
    const bool is_closed_loop,
    const bool move_end_nodes_in_span_direction,
    Real & c_coeff,
    Real & end_node_disp) const
{
  if (bd_side_list.empty())
    mooseError("The 'bd_side_list' argument of "
               "CircularBoundaryCorrectionGenerator::generateRadialCorrectionFactor is empty.");
  Real acc_area(0.0);
  Real acc_azi(0.0);
  std::vector<Real> d_azi_list;
  for (const auto & bd_side : bd_side_list)
  {
    const Point v1 = bd_side.first - circle_center;
    const Point v2 = bd_side.second - circle_center;
    // Use cross to calculate r * r * sin(d_azi_i) and then normalized by r * r to get sin(d_azi_i)
    acc_area += v1.cross(v2).norm() / v1.norm() / v2.norm();
    const Real azi1 = std::atan2(v1(1), v1(0));
    const Real azi2 = std::atan2(v2(1), v2(0));
    const Real d_azi =
        std::abs(azi1 - azi2) > M_PI ? 2 * M_PI - std::abs(azi1 - azi2) : std::abs(azi1 - azi2);
    acc_azi += d_azi;
    d_azi_list.push_back(d_azi);
  }

  if (!MooseUtils::absoluteFuzzyEqual(acc_azi, M_PI) && !is_closed_loop &&
      move_end_nodes_in_span_direction)
  {
    const Real k_1 = 1.0 + std::cos(acc_azi);
    const Real k_2 = acc_azi - std::sin(acc_azi);

    unsigned int ct = 0;
    Real diff = 1.0;
    Real c_old = 1.0;
    Real c_new = 1.0;
    while (diff >= libMesh::TOLERANCE && ct < 100)
    {
      c_old = c_new;
      const Real gc = k_2 + k_2 * std::cos(acc_azi * c_old) + k_1 * std::sin(acc_azi * c_old) -
                      k_1 * sineSummation(d_azi_list, c_old);
      const Real gcp = -k_2 * acc_azi * std::sin(acc_azi * c_old) +
                       k_1 * acc_azi * std::cos(acc_azi * c_old) -
                       k_1 * sinePrimeSummation(d_azi_list, c_old);
      c_new = c_old - gc / gcp;
      diff = std::abs(c_new - c_old);
      ct++;
    }

    if (ct >= 100)
      mooseError("Newton-Raphson method did not converge for generating the radial correction "
                 "factor for circular area preservation.");

    const Real norm_corr_factor = std::cos(0.5 * acc_azi) / std::cos(0.5 * acc_azi * c_new);
    end_node_disp = norm_corr_factor * std::sin(0.5 * acc_azi * c_new) - std::sin(0.5 * acc_azi);
    c_coeff = c_new;

    return norm_corr_factor;
  }
  end_node_disp = -1.0;
  c_coeff = 1.0;
  return std::sqrt(acc_azi / acc_area);
}

Real
CircularBoundaryCorrectionGenerator::sineSummation(const std::vector<Real> & th_list,
                                                   const Real coeff) const
{
  Real acc(0.0);
  for (const auto & th : th_list)
    acc += std::sin(th * coeff);
  return acc;
}

Real
CircularBoundaryCorrectionGenerator::sinePrimeSummation(const std::vector<Real> & th_list,
                                                        const Real coeff) const
{
  // Note that this calculates the derivative with regards to coeff
  Real acc(0.0);
  for (const auto & th : th_list)
    acc += th * std::cos(th * coeff);
  return acc;
}
