//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransitionLayerTools.h"
#include "MooseMeshUtils.h"
#include "MooseMesh.h"
#include "MeshGenerator.h"
#include "MooseError.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_quad4.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

namespace TransitionLayerTools
{
void
transitionLayerGenerator(ReplicatedMesh & mesh, // an empty mesh is expected
                         const std::vector<Point> boundary_points_vec_1,
                         std::vector<Point> boundary_points_vec_2,
                         const unsigned int num_layers,
                         const subdomain_id_type transition_layer_id,
                         const boundary_id_type input_boundary_1_id,
                         const boundary_id_type input_boundary_2_id,
                         const boundary_id_type begin_side_boundary_id,
                         const boundary_id_type end_side_boundary_id,
                         const std::string type,
                         const std::string name,
                         const bool quad_elem,
                         const Real bias_parameter,
                         const Real sigma)
{
  if (!isXYPlane(boundary_points_vec_1) || !isXYPlane(boundary_points_vec_2))
    mooseError("In ",
               type,
               " ",
               name,
               ", the input vectors of points for "
               "TransitionLayerTools::transitionLayerGenerator "
               "must be in XY plane.");

  const unsigned int vec_1_node_num = boundary_points_vec_1.size();
  const unsigned int vec_2_node_num = boundary_points_vec_2.size();

  if (vec_1_node_num < 2 || vec_2_node_num < 2)
    mooseError("In ",
               type,
               " ",
               name,
               ", the two input vectors of points for "
               "TransitionLayerTools::transitionLayerGenerator "
               "must respectively contain at least two elements.");

  if (quad_elem && boundary_points_vec_1.size() != boundary_points_vec_2.size())
    mooseError("In ",
               type,
               " ",
               name,
               ", QUAD4 elements option can only be selected when the two input vectors of Points "
               "have the same length.");

  if (needFlip(boundary_points_vec_1, boundary_points_vec_2))
  {
    std::reverse(boundary_points_vec_2.begin(), boundary_points_vec_2.end());
    mooseWarning("In TransitionLayerTools, one of the vector of Points must be flipped to ensure "
                 "correct transition layer shape.");
  }

  std::vector<Real> vec_1_index; // Unweighted index
  std::vector<Real> vec_2_index; // Unweighted index

  std::vector<Real> wt_1;
  std::vector<Real> index_1; // Weighted index
  std::vector<Real> wt_2;
  std::vector<Real> index_2; // Weighted index

  // create interpolations
  std::unique_ptr<LinearInterpolation> linear_vec_1_x;
  std::unique_ptr<LinearInterpolation> linear_vec_1_y;
  std::unique_ptr<SplineInterpolation> spline_vec_1_l;
  std::unique_ptr<LinearInterpolation> linear_vec_2_x;
  std::unique_ptr<LinearInterpolation> linear_vec_2_y;
  std::unique_ptr<SplineInterpolation> spline_vec_2_l;

  weightedInterpolator(vec_1_node_num,
                       boundary_points_vec_1,
                       vec_1_index,
                       wt_1,
                       index_1,
                       sigma,
                       linear_vec_1_x,
                       linear_vec_1_y,
                       spline_vec_1_l);
  weightedInterpolator(vec_2_node_num,
                       boundary_points_vec_2,
                       vec_2_index,
                       wt_2,
                       index_2,
                       sigma,
                       linear_vec_2_x,
                       linear_vec_2_y,
                       spline_vec_2_l);

  // If the two input vectors have different sizes
  // The node numbers of the intermediate layers change linearly
  const Real increment = ((Real)vec_2_node_num - (Real)vec_1_node_num) / (Real)(num_layers);
  // Number of nodes in each sublayer
  std::vector<unsigned int> node_number_vec;
  // 2D vector of nodes
  std::vector<std::vector<Node *>> nodes(num_layers + 1);
  // Node counter
  unsigned int node_counter = 0;

  for (unsigned int i = 0; i < num_layers + 1; i++)
  {
    // calculate number of nodes in each sublayer
    node_number_vec.push_back(vec_1_node_num +
                              (unsigned int)(increment * i + 0.5 - (increment < 0)));
    // Reserve memory for new nodes
    nodes[i] = std::vector<Node *>(node_number_vec[i]);

    // Calculate vectors of weighted surrogated index for side #1
    std::vector<Real> weighted_surrogate_index_1;
    std::vector<Real> unweighted_surrogate_index_1;

    surrogateGenerator(weighted_surrogate_index_1,
                       unweighted_surrogate_index_1,
                       node_number_vec,
                       wt_1,
                       vec_1_index,
                       vec_1_node_num,
                       i);

    // Calculate vectors of weighted surrogated index for side #2
    std::vector<Real> weighted_surrogate_index_2;
    std::vector<Real> unweighted_surrogate_index_2;

    surrogateGenerator(weighted_surrogate_index_2,
                       unweighted_surrogate_index_2,
                       node_number_vec,
                       wt_2,
                       vec_2_index,
                       vec_2_node_num,
                       i);

    for (unsigned int j = 0; j < node_number_vec[i]; j++)
    {
      // Create surrogate Points on side #1 for Point #j on the sublayer
      Point surrogate_pos_1 = Point(linear_vec_1_x->sample(weighted_surrogate_index_1[j]),
                                    linear_vec_1_y->sample(weighted_surrogate_index_1[j]),
                                    0.0);
      // Create surrogate Points on side #2 for Point #j on the sublayer
      Point surrogate_pos_2 = Point(linear_vec_2_x->sample(weighted_surrogate_index_2[j]),
                                    linear_vec_2_y->sample(weighted_surrogate_index_2[j]),
                                    0.0);
      const Real l_ratio = bias_parameter <= 0.0
                               ? std::pow(spline_vec_2_l->sample(weighted_surrogate_index_2[j]) /
                                              spline_vec_1_l->sample(weighted_surrogate_index_1[j]),
                                          1.0 / ((Real)num_layers - 1.0))
                               : bias_parameter;
      const Real index_factor =
          MooseUtils::absoluteFuzzyEqual(l_ratio, 1.0)
              ? (Real)i / (Real)num_layers
              : (1.0 - std::pow(l_ratio, (Real)i)) / (1.0 - std::pow(l_ratio, (Real)num_layers));
      Point tmp_point = surrogate_pos_2 * index_factor + surrogate_pos_1 * (1.0 - index_factor);
      nodes[i][j] = mesh.add_point(tmp_point, j + node_counter);
    }
    node_counter += node_number_vec[i];
  }
  // Create triangular elements based on the 2D Node vector
  if (quad_elem)
    elementsCreationFromNodesVectorsQuad(mesh,
                                         nodes,
                                         num_layers,
                                         node_number_vec,
                                         transition_layer_id,
                                         input_boundary_1_id,
                                         input_boundary_2_id,
                                         begin_side_boundary_id,
                                         end_side_boundary_id);
  else
    elementsCreationFromNodesVectors(mesh,
                                     nodes,
                                     num_layers,
                                     node_number_vec,
                                     transition_layer_id,
                                     input_boundary_1_id,
                                     input_boundary_2_id,
                                     begin_side_boundary_id,
                                     end_side_boundary_id);
}

void
transitionLayerGenerator(ReplicatedMesh & mesh,
                         const std::vector<Point> boundary_points_vec_1,
                         const std::vector<Point> boundary_points_vec_2,
                         const unsigned int num_layers,
                         const subdomain_id_type transition_layer_id,
                         const boundary_id_type external_boundary_id,
                         const std::string type,
                         const std::string name,
                         const bool quad_elem)
{
  transitionLayerGenerator(mesh,
                           boundary_points_vec_1,
                           boundary_points_vec_2,
                           num_layers,
                           transition_layer_id,
                           external_boundary_id,
                           external_boundary_id,
                           external_boundary_id,
                           external_boundary_id,
                           type,
                           name,
                           quad_elem);
}

void
elementsCreationFromNodesVectorsQuad(ReplicatedMesh & mesh,
                                     const std::vector<std::vector<Node *>> nodes,
                                     const unsigned int num_layers,
                                     const std::vector<unsigned int> node_number_vec,
                                     const subdomain_id_type transition_layer_id,
                                     const boundary_id_type input_boundary_1_id,
                                     const boundary_id_type input_boundary_2_id,
                                     const boundary_id_type begin_side_boundary_id,
                                     const boundary_id_type end_side_boundary_id)
{
  const unsigned int node_number = node_number_vec.front();

  for (unsigned int i = 0; i < num_layers; i++)
    for (unsigned int j = 1; j < node_number; j++)
    {
      Elem * elem = mesh.add_elem(new Quad4);
      elem->set_node(0) = nodes[i][j - 1];
      elem->set_node(1) = nodes[i + 1][j - 1];
      elem->set_node(2) = nodes[i + 1][j];
      elem->set_node(3) = nodes[i][j];
      elem->subdomain_id() = transition_layer_id;
      if (i == 0)
        mesh.boundary_info->add_side(elem, 3, input_boundary_1_id);
      if (i == num_layers - 1)
        mesh.boundary_info->add_side(elem, 1, input_boundary_2_id);
      if (j == 1)
        mesh.boundary_info->add_side(elem, 0, begin_side_boundary_id);
      if (j == node_number - 1)
        mesh.boundary_info->add_side(elem, 2, end_side_boundary_id);
    }
}

void
elementsCreationFromNodesVectors(ReplicatedMesh & mesh,
                                 const std::vector<std::vector<Node *>> nodes,
                                 const unsigned int num_layers,
                                 const std::vector<unsigned int> node_number_vec,
                                 const subdomain_id_type transition_layer_id,
                                 const boundary_id_type input_boundary_1_id,
                                 const boundary_id_type input_boundary_2_id,
                                 const boundary_id_type begin_side_boundary_id,
                                 const boundary_id_type end_side_boundary_id)
{
  for (unsigned int i = 0; i < num_layers; i++)
  {
    unsigned int nodes_up_it = 0;
    unsigned int nodes_down_it = 0;
    const unsigned int node_number_up = node_number_vec[i + 1];
    const unsigned int node_number_down = node_number_vec[i];

    while (nodes_up_it < node_number_up - 1 && nodes_down_it < node_number_down - 1 &&
           nodes_up_it + nodes_down_it < node_number_up + node_number_down - 3)
    {
      // Define the two possible options and chose the one with shorter distance
      Real dis1 = (*nodes[i + 1][nodes_up_it] - *nodes[i][nodes_down_it + 1]).norm();
      Real dis2 = (*nodes[i + 1][nodes_up_it + 1] - *nodes[i][nodes_down_it]).norm();
      if (dis1 > dis2)
      {
        Elem * elem = mesh.add_elem(new Tri3);
        elem->set_node(0) = nodes[i + 1][nodes_up_it];
        elem->set_node(1) = nodes[i][nodes_down_it];
        elem->set_node(2) = nodes[i + 1][nodes_up_it + 1];
        elem->subdomain_id() = transition_layer_id;
        if (i == num_layers - 1)
          mesh.boundary_info->add_side(elem, 2, input_boundary_2_id);
        if (nodes_up_it == 0 && nodes_down_it == 0)
          mesh.boundary_info->add_side(elem, 0, begin_side_boundary_id);
        nodes_up_it++;
      }
      else
      {
        Elem * elem = mesh.add_elem(new Tri3);
        elem->set_node(0) = nodes[i + 1][nodes_up_it];
        elem->set_node(1) = nodes[i][nodes_down_it];
        elem->set_node(2) = nodes[i][nodes_down_it + 1];
        elem->subdomain_id() = transition_layer_id;
        if (i == 0)
          mesh.boundary_info->add_side(elem, 1, input_boundary_1_id);
        if (nodes_up_it == 0 && nodes_down_it == 0)
          mesh.boundary_info->add_side(elem, 0, begin_side_boundary_id);
        nodes_down_it++;
      }
    }
    // Handle the end
    while (nodes_up_it < node_number_up - 1)
    {
      Elem * elem = mesh.add_elem(new Tri3);
      elem->set_node(0) = nodes[i + 1][nodes_up_it];
      elem->set_node(1) = nodes[i][nodes_down_it];
      elem->set_node(2) = nodes[i + 1][nodes_up_it + 1];
      elem->subdomain_id() = transition_layer_id;
      nodes_up_it++;
      if (i == num_layers - 1)
        mesh.boundary_info->add_side(elem, 2, input_boundary_2_id);
      if (nodes_up_it == node_number_up - 1 && nodes_down_it == node_number_down - 1)
        mesh.boundary_info->add_side(elem, 1, end_side_boundary_id);
    }
    while (nodes_down_it < node_number_down - 1)
    {
      Elem * elem = mesh.add_elem(new Tri3);
      elem->set_node(0) = nodes[i + 1][nodes_up_it];
      elem->set_node(1) = nodes[i][nodes_down_it];
      elem->set_node(2) = nodes[i][nodes_down_it + 1];
      elem->subdomain_id() = transition_layer_id;
      nodes_down_it++;
      if (i == 0)
        mesh.boundary_info->add_side(elem, 1, input_boundary_1_id);
      if (nodes_up_it == node_number_up - 1 && nodes_down_it == node_number_down - 1)
        mesh.boundary_info->add_side(elem, 2, end_side_boundary_id);
    }
  }
}

void
weightedInterpolator(const unsigned int vec_node_num,
                     const std::vector<Point> boundary_points_vec,
                     std::vector<Real> & vec_index,
                     std::vector<Real> & wt,
                     std::vector<Real> & index,
                     const Real sigma,
                     std::unique_ptr<LinearInterpolation> & linear_vec_x,
                     std::unique_ptr<LinearInterpolation> & linear_vec_y,
                     std::unique_ptr<SplineInterpolation> & spline_vec_l)
{
  std::vector<Real> pos_x;
  std::vector<Real> pos_y;
  std::vector<Real> dist_vec;
  std::vector<Real> pos_l;

  for (unsigned int i = 0; i < vec_node_num; i++)
  {
    // Unweighted, the index interval is just uniform
    // Normalized range 0~1
    vec_index.push_back((Real)i / ((Real)vec_node_num - 1.0));
    // X and Y coordinates cooresponding to the index
    pos_x.push_back(boundary_points_vec[i](0));
    pos_y.push_back(boundary_points_vec[i](1));
    // Use Point-to-Point distance as unnormalized weight
    if (i > 0)
    {
      wt.push_back((boundary_points_vec[i] - boundary_points_vec[i - 1]).norm());
      dist_vec.push_back(wt.back());
    }
    // Accumulated unnormalized weights to get unnormalized weighted index
    index.push_back(std::accumulate(wt.begin(), wt.end(), 0.0));
  }
  const Real dist_vec_total = index.back(); // Total accumulated distances
  const Real wt_norm_factor = dist_vec_total / ((Real)vec_node_num - 1.0); // Normalization factor
  // Normalization for both weights and weighted indices
  std::transform(
      wt.begin(), wt.end(), wt.begin(), [wt_norm_factor](Real & c) { return c / wt_norm_factor; });
  std::transform(index.begin(),
                 index.end(),
                 index.begin(),
                 [dist_vec_total](Real & c) { return c / dist_vec_total; });
  // Use Gaussian blurring to smoothen local density
  for (unsigned int i = 0; i < vec_node_num; i++)
  {
    Real gaussian_factor(0.0);
    Real sum_tmp(0.0);
    // Use interval as parameter now, consider distance in the future
    for (unsigned int j = 0; j < vec_node_num - 1; j++)
    {
      // dis_vec and index are off by 0.5
      const Real tmp_factor =
          exp(-((Real)(i - j) - 0.5) * ((Real)(i - j) - 0.5) / 2.0 / sigma / sigma);
      gaussian_factor += tmp_factor;
      sum_tmp += tmp_factor * dist_vec[j];
    }
    pos_l.push_back(sum_tmp / gaussian_factor);
  }
  // Interpolate positions based on weighted indices
  linear_vec_x = libmesh_make_unique<LinearInterpolation>(index, pos_x);
  linear_vec_y = libmesh_make_unique<LinearInterpolation>(index, pos_y);
  spline_vec_l = libmesh_make_unique<SplineInterpolation>(index, pos_l);
}

void
surrogateGenerator(std::vector<Real> & weighted_surrogate_index,
                   std::vector<Real> & unweighted_surrogate_index,
                   const std::vector<unsigned int> node_number_vec,
                   const std::vector<Real> wt,
                   const std::vector<Real> index,
                   const unsigned int boundary_node_num,
                   const unsigned int i)
{
  // First element is trivial
  weighted_surrogate_index.push_back(0.0);
  unweighted_surrogate_index.push_back(0.0);
  for (unsigned int j = 1; j < node_number_vec[i]; j++)
  {
    // uniform interval for unweighted index
    unweighted_surrogate_index.push_back((Real)j / ((Real)node_number_vec[i] - 1.0));
    // >
    const auto it_0 =
        std::upper_bound(index.begin(), index.end(), unweighted_surrogate_index[j - 1]);
    // >=
    const auto it_1 = std::lower_bound(index.begin(), index.end(), unweighted_surrogate_index[j]);
    //
    const auto it_dist = std::distance(it_0, it_1);
    //
    const auto it_start = std::distance(index.begin(), it_0);

    if (it_0 == it_1)
      weighted_surrogate_index.push_back(weighted_surrogate_index[j - 1] +
                                         wt[it_start - 1] / ((Real)node_number_vec[i] - 1.0));
    else
    {
      weighted_surrogate_index.push_back(weighted_surrogate_index[j - 1]);
      weighted_surrogate_index[j] += (*it_0 - unweighted_surrogate_index[j - 1]) * wt[it_start - 1];
      weighted_surrogate_index[j] +=
          (unweighted_surrogate_index[j] - *(it_1 - 1)) * wt[it_start + it_dist - 1];
      for (unsigned int k = 1; k < it_dist; k++)
        weighted_surrogate_index[j] += wt[it_start + k - 1] / ((Real)boundary_node_num - 1.0);
    }
  }
}

bool
isXYPlane(const std::vector<Point> vec_pts)
{
  for (auto it = vec_pts.begin(); it != vec_pts.end(); it++)
    if (!MooseUtils::absoluteFuzzyEqual((*it)(2), 0.0))
      return false;
  return true;
}

bool
needFlip(const std::vector<Point> vec_pts_1, const std::vector<Point> vec_pts_2)
{
  const Real th1 =
      acos((vec_pts_1.back() - vec_pts_1.front()) * (vec_pts_2.front() - vec_pts_1.front()) /
           (vec_pts_1.back() - vec_pts_1.front()).norm() /
           (vec_pts_2.front() - vec_pts_1.front()).norm());
  const Real th2 = acos(
      (vec_pts_2.back() - vec_pts_1.back()) * (vec_pts_1.front() - vec_pts_1.back()) /
      (vec_pts_2.back() - vec_pts_1.back()).norm() / (vec_pts_1.front() - vec_pts_1.back()).norm());
  const Real th3 = acos(
      (vec_pts_2.front() - vec_pts_2.back()) * (vec_pts_1.back() - vec_pts_2.back()) /
      (vec_pts_2.front() - vec_pts_2.back()).norm() / (vec_pts_1.back() - vec_pts_2.back()).norm());
  const Real th4 =
      acos((vec_pts_1.front() - vec_pts_2.front()) * (vec_pts_2.back() - vec_pts_2.front()) /
           (vec_pts_1.front() - vec_pts_2.front()).norm() /
           (vec_pts_2.back() - vec_pts_2.front()).norm());
  if (MooseUtils::absoluteFuzzyEqual(th1 + th2 + th3 + th4, 2 * M_PI))
    return false;
  return true;
}

bool
isBoundaryValid(ReplicatedMesh & mesh,
                Real & max_node_radius,
                unsigned short & invalid_type,
                std::vector<dof_id_type> & boundary_ordered_node_list,
                const Point origin_pt,
                const boundary_id_type bid)
{
  max_node_radius = 0.0;
  invalid_type = 0;
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto side_list_tmp = boundary_info.build_side_list();
  unsigned int elem_counter = 0;
  std::vector<std::pair<dof_id_type, dof_id_type>> boundary_node_assm;
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
    auto isMatch1 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.first == end_node_id; };
    auto isMatch2 = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.second == end_node_id; };
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
  // If the boundary_ordered_node_list front and back are not the same, the boundary is not a loop.
  // This is not done inside the loop just for some potential applications in the future.
  if (boundary_ordered_node_list.front() != boundary_ordered_node_list.back())
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
isExternalBoundary(ReplicatedMesh & mesh, const boundary_id_type bid)
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
}
