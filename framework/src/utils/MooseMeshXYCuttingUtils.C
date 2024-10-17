//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseMeshXYCuttingUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_base.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/face_tri3.h"

using namespace libMesh;

namespace MooseMeshXYCuttingUtils
{

void
lineRemoverMoveNode(ReplicatedMesh & mesh,
                    const std::vector<Real> & bdry_pars,
                    const subdomain_id_type block_id_to_remove,
                    const std::set<subdomain_id_type> & subdomain_ids_set,
                    const boundary_id_type trimming_section_boundary_id,
                    const boundary_id_type external_boundary_id,
                    const std::vector<boundary_id_type> & other_boundaries_to_conform,
                    const bool assign_ext_to_new,
                    const bool side_to_remove)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Only select the boundaries_to_conform
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> slc_bdry_side_list;
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
    if (std::get<2>(bdry_side_list[i]) == external_boundary_id ||
        std::find(other_boundaries_to_conform.begin(),
                  other_boundaries_to_conform.end(),
                  std::get<2>(bdry_side_list[i])) != other_boundaries_to_conform.end())
      slc_bdry_side_list.push_back(bdry_side_list[i]);

  // Assign block id for elements to be removed
  // Also record the elements crossed by the line and with its average vertices on the removal side
  std::vector<dof_id_type> crossed_elems_to_remove;
  for (auto elem_it = mesh.active_elements_begin(); elem_it != mesh.active_elements_end();
       elem_it++)
  {
    // Check all the vertices of the element
    unsigned short removal_side_count = 0;
    for (unsigned int i = 0; i < (*elem_it)->n_vertices(); i++)
    {
      // First check if the vertex is on the XY-Plane
      if (!MooseUtils::absoluteFuzzyEqual((*elem_it)->point(i)(2), 0.0))
        mooseError(
            "MooseMeshXYCuttingUtils::lineRemoverMoveNode() only works for 2D meshes in XY plane.");
      if (lineSideDeterminator((*elem_it)->point(i)(0),
                               (*elem_it)->point(i)(1),
                               bdry_pars[0],
                               bdry_pars[1],
                               bdry_pars[2],
                               side_to_remove))
        removal_side_count++;
    }
    if (removal_side_count == (*elem_it)->n_vertices())
    {
      (*elem_it)->subdomain_id() = block_id_to_remove;
      continue;
    }
    // Check the average of the vertices of the element
    if (lineSideDeterminator((*elem_it)->vertex_average()(0),
                             (*elem_it)->vertex_average()(1),
                             bdry_pars[0],
                             bdry_pars[1],
                             bdry_pars[2],
                             side_to_remove))
      crossed_elems_to_remove.push_back((*elem_it)->id());
  }
  // Check each crossed element to see if removing it would lead to boundary moving
  for (const auto & elem_id : crossed_elems_to_remove)
  {
    bool remove_flag = true;
    for (unsigned int i = 0; i < mesh.elem_ptr(elem_id)->n_sides(); i++)
    {
      if (mesh.elem_ptr(elem_id)->neighbor_ptr(i) != nullptr)
        if (mesh.elem_ptr(elem_id)->neighbor_ptr(i)->subdomain_id() != block_id_to_remove &&
            std::find(crossed_elems_to_remove.begin(),
                      crossed_elems_to_remove.end(),
                      mesh.elem_ptr(elem_id)->neighbor_ptr(i)->id()) ==
                crossed_elems_to_remove.end())
        {
          if (mesh.elem_ptr(elem_id)->subdomain_id() !=
              mesh.elem_ptr(elem_id)->neighbor_ptr(i)->subdomain_id())
          {
            remove_flag = false;
            break;
          }
        }
    }
    if (remove_flag)
      mesh.elem_ptr(elem_id)->subdomain_id() = block_id_to_remove;
  }

  // Identify all the nodes that are on the interface between block_id_to_remove and other blocks
  // !!! We need a check here: if a node is on the retaining side, the removed element has a
  // different subdomain id
  std::vector<dof_id_type> node_list;
  for (auto elem_it = mesh.active_subdomain_set_elements_begin(subdomain_ids_set);
       elem_it != mesh.active_subdomain_set_elements_end(subdomain_ids_set);
       elem_it++)
  {
    for (unsigned int i = 0; i < (*elem_it)->n_sides(); i++)
    {
      if ((*elem_it)->neighbor_ptr(i) != nullptr)
        if ((*elem_it)->neighbor_ptr(i)->subdomain_id() == block_id_to_remove)
        {
          node_list.push_back((*elem_it)->side_ptr(i)->node_ptr(0)->id());
          node_list.push_back((*elem_it)->side_ptr(i)->node_ptr(1)->id());
          boundary_info.add_side(*elem_it, i, trimming_section_boundary_id);
          if (assign_ext_to_new && trimming_section_boundary_id != external_boundary_id)
            boundary_info.add_side(*elem_it, i, external_boundary_id);
        }
    }
  }
  // Remove duplicate nodes
  const auto unique_it = std::unique(node_list.begin(), node_list.end());
  node_list.resize(std::distance(node_list.begin(), unique_it));
  // Mark those nodes that are on a boundary that requires conformality
  // If both nodes of a side are involved, we should only move one node
  std::vector<bool> node_list_flag(node_list.size(), false);
  std::vector<Point> node_list_point(node_list.size(), Point(0.0, 0.0, 0.0));
  // Loop over all the selected sides
  for (unsigned int i = 0; i < slc_bdry_side_list.size(); i++)
  {
    // Get the two node ids of the side
    dof_id_type side_id_0 = mesh.elem_ptr(std::get<0>(slc_bdry_side_list[i]))
                                ->side_ptr(std::get<1>(slc_bdry_side_list[i]))
                                ->node_ptr(0)
                                ->id();
    dof_id_type side_id_1 = mesh.elem_ptr(std::get<0>(slc_bdry_side_list[i]))
                                ->side_ptr(std::get<1>(slc_bdry_side_list[i]))
                                ->node_ptr(1)
                                ->id();
    // True means the selected bdry node is in the node list of the trimming interface
    bool side_id_0_in =
        !(std::find(node_list.begin(), node_list.end(), side_id_0) == node_list.end());
    bool side_id_1_in =
        !(std::find(node_list.begin(), node_list.end(), side_id_1) == node_list.end());

    // True means the selected bdry node is on the removal side of the trimming interface
    bool side_node_0_remove = lineSideDeterminator((*mesh.node_ptr(side_id_0))(0),
                                                   (*mesh.node_ptr(side_id_0))(1),
                                                   bdry_pars[0],
                                                   bdry_pars[1],
                                                   bdry_pars[2],
                                                   side_to_remove);
    bool side_node_1_remove = lineSideDeterminator((*mesh.node_ptr(side_id_1))(0),
                                                   (*mesh.node_ptr(side_id_1))(1),
                                                   bdry_pars[0],
                                                   bdry_pars[1],
                                                   bdry_pars[2],
                                                   side_to_remove);
    // If both nodes of that side are involved in the trimming interface
    if (side_id_0_in && side_id_1_in)
      // The side needs to be removed from the sideset because it is not longer an interface
      // The other node will be handled by other element's side
      boundary_info.remove_side(mesh.elem_ptr(std::get<0>(slc_bdry_side_list[i])),
                                std::get<1>(slc_bdry_side_list[i]),
                                std::get<2>(slc_bdry_side_list[i]));
    // If node 0 is on the trimming interface, and the side is cut by the trimming line
    else if (side_id_0_in && (side_node_0_remove != side_node_1_remove))
    {
      // Use the intersection point as the destination of the node after moving
      node_list_flag[std::distance(
          node_list.begin(), std::find(node_list.begin(), node_list.end(), side_id_0))] = true;
      const Point p0 = *mesh.node_ptr(side_id_0);
      const Point p1 = *mesh.node_ptr(side_id_1);

      node_list_point[std::distance(node_list.begin(),
                                    std::find(node_list.begin(), node_list.end(), side_id_0))] =
          twoPointandLineIntersection(p0, p1, bdry_pars[0], bdry_pars[1], bdry_pars[2]);
    }
    // If node 1 is on the trimming interface, and the side is cut by the trimming line
    else if (side_id_1_in && (side_node_0_remove != side_node_1_remove))
    {
      // Use the intersection point as the destination of the node after moving
      node_list_flag[std::distance(
          node_list.begin(), std::find(node_list.begin(), node_list.end(), side_id_1))] = true;
      const Point p0 = *mesh.node_ptr(side_id_0);
      const Point p1 = *mesh.node_ptr(side_id_1);

      node_list_point[std::distance(node_list.begin(),
                                    std::find(node_list.begin(), node_list.end(), side_id_1))] =
          twoPointandLineIntersection(p0, p1, bdry_pars[0], bdry_pars[1], bdry_pars[2]);
    }
  }

  // move nodes
  for (unsigned int i = 0; i < node_list.size(); i++)
  {
    // This means the node is on both the trimming boundary and the original external
    // boundary/selected interface boundaries. In order to keep the shape of the original external
    // boundary, the node is moved along the original external boundary.
    if (node_list_flag[i])
      *(mesh.node_ptr(node_list[i])) = node_list_point[i];
    // This means the node does not need to conform to any boundaries.
    // Just move it along the normal direction of the trimming line.
    else
    {
      const Real x0 = (*(mesh.node_ptr(node_list[i])))(0);
      const Real y0 = (*(mesh.node_ptr(node_list[i])))(1);
      (*(mesh.node_ptr(node_list[i])))(0) =
          (bdry_pars[1] * (bdry_pars[1] * x0 - bdry_pars[0] * y0) - bdry_pars[0] * bdry_pars[2]) /
          (bdry_pars[0] * bdry_pars[0] + bdry_pars[1] * bdry_pars[1]);
      (*(mesh.node_ptr(node_list[i])))(1) =
          (bdry_pars[0] * (-bdry_pars[1] * x0 + bdry_pars[0] * y0) - bdry_pars[1] * bdry_pars[2]) /
          (bdry_pars[0] * bdry_pars[0] + bdry_pars[1] * bdry_pars[1]);
    }
  }

  // Delete the block
  for (auto elem_it = mesh.active_subdomain_elements_begin(block_id_to_remove);
       elem_it != mesh.active_subdomain_elements_end(block_id_to_remove);
       elem_it++)
    mesh.delete_elem(*elem_it);
  mesh.contract();
  mesh.find_neighbors();
  // Delete zero volume elements
  std::vector<dof_id_type> zero_elems;
  for (auto elem_it = mesh.elements_begin(); elem_it != mesh.elements_end(); elem_it++)
  {
    if (MooseUtils::absoluteFuzzyEqual((*elem_it)->volume(), 0.0))
    {
      for (unsigned int i = 0; i < (*elem_it)->n_sides(); i++)
      {
        if ((*elem_it)->neighbor_ptr(i) != nullptr)
        {
          boundary_info.add_side((*elem_it)->neighbor_ptr(i),
                                 ((*elem_it)->neighbor_ptr(i))->which_neighbor_am_i(*elem_it),
                                 external_boundary_id);
          boundary_info.add_side((*elem_it)->neighbor_ptr(i),
                                 ((*elem_it)->neighbor_ptr(i))->which_neighbor_am_i(*elem_it),
                                 trimming_section_boundary_id);
        }
      }
      zero_elems.push_back((*elem_it)->id());
    }
  }
  for (const auto & zero_elem : zero_elems)
    mesh.delete_elem(mesh.elem_ptr(zero_elem));
  mesh.contract();
  // As we modified the side_list, it is safer to clear the node_list
  boundary_info.clear_boundary_node_ids();
  mesh.prepare_for_use();
}

bool
lineSideDeterminator(const Real px,
                     const Real py,
                     const Real param_1,
                     const Real param_2,
                     const Real param_3,
                     const bool direction_param,
                     const Real dis_tol)
{
  const Real tmp = px * param_1 + py * param_2 + param_3;
  return direction_param ? tmp >= dis_tol : tmp <= dis_tol;
}

Point
twoLineIntersection(const Real param_11,
                    const Real param_12,
                    const Real param_13,
                    const Real param_21,
                    const Real param_22,
                    const Real param_23)
{
  return Point(
      (param_12 * param_23 - param_22 * param_13) / (param_11 * param_22 - param_21 * param_12),
      (param_13 * param_21 - param_23 * param_11) / (param_11 * param_22 - param_21 * param_12),
      0.0);
}

Point
twoPointandLineIntersection(const Point & pt1,
                            const Point & pt2,
                            const Real param_1,
                            const Real param_2,
                            const Real param_3)
{
  return twoLineIntersection(param_1,
                             param_2,
                             param_3,
                             pt2(1) - pt1(1),
                             pt1(0) - pt2(0),
                             pt2(0) * pt1(1) - pt1(0) * pt2(1));
}

bool
quasiTriElementsFixer(ReplicatedMesh & mesh,
                      const std::set<subdomain_id_type> & subdomain_ids_set,
                      const subdomain_id_type tri_elem_subdomain_shift,
                      const SubdomainName tri_elem_subdomain_name_suffix)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  // Define the subdomain id shift for the new TRI3 element subdomain(s)
  const subdomain_id_type max_subdomain_id(*subdomain_ids_set.rbegin());
  const subdomain_id_type tri_subdomain_id_shift =
      tri_elem_subdomain_shift == Moose::INVALID_BLOCK_ID ? max_subdomain_id
                                                          : tri_elem_subdomain_shift;
  mooseAssert(std::numeric_limits<subdomain_id_type>::max() - max_subdomain_id >
                  tri_subdomain_id_shift,
              "The TRI elements subdomain id to be assigned may exceed the numeric limit.");
  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  std::vector<std::tuple<Elem *, unsigned int, bool, bool>> bad_elems_rec;
  // Loop over all the active elements to find any degenerate QUAD elements
  for (auto & elem : as_range(mesh.active_elements_begin(), mesh.active_elements_end()))
  {
    // Two types of degenerate QUAD elements are identified:
    // (1) QUAD elements with three collinear vertices
    // (2) QUAD elements with two overlapped vertices
    const auto elem_angles = vertex_angles(*elem);
    const auto elem_distances = vertex_distances(*elem);
    // Type 1
    if (MooseUtils::absoluteFuzzyEqual(elem_angles.front().first, M_PI, 0.001))
    {
      bad_elems_rec.push_back(std::make_tuple(elem, elem_angles.front().second, false, true));
      continue;
    }
    // Type 2
    if (MooseUtils::absoluteFuzzyEqual(elem_distances.front().first, 0.0))
    {
      bad_elems_rec.push_back(std::make_tuple(elem, elem_distances.front().second, false, false));
    }
  }
  std::set<subdomain_id_type> new_subdomain_ids;
  // Loop over all the identified degenerate QUAD elements
  for (const auto & bad_elem : bad_elems_rec)
  {
    std::vector<boundary_id_type> elem_bdry_container_0;
    std::vector<boundary_id_type> elem_bdry_container_1;
    std::vector<boundary_id_type> elem_bdry_container_2;

    Elem * elem_0 = std::get<0>(bad_elem);
    if (std::get<3>(bad_elem))
    {
      // elems 1 and 2 are the neighboring elements of the degenerate element corresponding to the
      // two collinear sides.
      // For the degenerated element with three colinear vertices, if the elems 2 and 3 do not
      // exist, the two sides are on the external boundary formed by trimming.
      Elem * elem_1 = elem_0->neighbor_ptr(std::get<1>(bad_elem));
      Elem * elem_2 = elem_0->neighbor_ptr((std::get<1>(bad_elem) - 1) % elem_0->n_vertices());
      if ((elem_1 != nullptr || elem_2 != nullptr))
        throw MooseException("The input mesh has degenerate quad element before trimming.");
    }
    mesh.get_boundary_info().boundary_ids(
        elem_0, (std::get<1>(bad_elem) + 1) % elem_0->n_vertices(), elem_bdry_container_0);
    mesh.get_boundary_info().boundary_ids(
        elem_0, (std::get<1>(bad_elem) + 2) % elem_0->n_vertices(), elem_bdry_container_1);
    mesh.get_boundary_info().boundary_ids(
        elem_0, (std::get<1>(bad_elem) + 3) % elem_0->n_vertices(), elem_bdry_container_2);

    // Record subdomain id of the degenerate element
    auto elem_block_id = elem_0->subdomain_id();
    // Define the three of four nodes that will be used to generate the TRI element
    auto pt0 = elem_0->node_ptr((std::get<1>(bad_elem) + 1) % elem_0->n_vertices());
    auto pt1 = elem_0->node_ptr((std::get<1>(bad_elem) + 2) % elem_0->n_vertices());
    auto pt2 = elem_0->node_ptr((std::get<1>(bad_elem) + 3) % elem_0->n_vertices());
    // Record all the element extra integers of the degenerate element
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
      exist_extra_ids[j] = elem_0->get_extra_integer(j);
    // Delete the degenerate QUAD element
    mesh.delete_elem(elem_0);
    // Create the new TRI element
    Elem * elem_Tri3 = mesh.add_elem(new Tri3);
    elem_Tri3->set_node(0) = pt0;
    elem_Tri3->set_node(1) = pt1;
    elem_Tri3->set_node(2) = pt2;
    // Retain the boundary information
    for (auto bdry_id : elem_bdry_container_0)
      boundary_info.add_side(elem_Tri3, 0, bdry_id);
    for (auto bdry_id : elem_bdry_container_1)
      boundary_info.add_side(elem_Tri3, 1, bdry_id);
    for (auto bdry_id : elem_bdry_container_2)
      boundary_info.add_side(elem_Tri3, 2, bdry_id);
    // Assign subdomain id for the TRI element by shifting its original subdomain id
    elem_Tri3->subdomain_id() = elem_block_id + tri_subdomain_id_shift;
    new_subdomain_ids.emplace(elem_block_id + tri_subdomain_id_shift);
    // Retain element extra integers
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
      elem_Tri3->set_extra_integer(j, exist_extra_ids[j]);
  }
  // Assign subdomain names for the new TRI elements
  for (auto & nid : new_subdomain_ids)
  {
    const SubdomainName old_name = mesh.subdomain_name(nid - tri_subdomain_id_shift);
    if (MooseMeshUtils::getSubdomainID(
            (old_name.empty() ? (SubdomainName)(std::to_string(nid - tri_subdomain_id_shift))
                              : old_name) +
                "_" + tri_elem_subdomain_name_suffix,
            mesh) != Moose::INVALID_BLOCK_ID)
      throw MooseException("The new subdomain name already exists in the mesh.");
    mesh.subdomain_name(nid) =
        (old_name.empty() ? (SubdomainName)(std::to_string(nid - tri_subdomain_id_shift))
                          : old_name) +
        "_" + tri_elem_subdomain_name_suffix;
    mooseWarning("Degenerate QUAD elements have been converted into TRI elements with a new "
                 "subdomain name: " +
                 mesh.subdomain_name(nid) + ".");
  }
  return bad_elems_rec.size();
}

std::vector<std::pair<Real, unsigned int>>
vertex_angles(const Elem & elem)
{
  std::vector<std::pair<Real, unsigned int>> angles;
  const unsigned int n_vertices = elem.n_vertices();

  for (unsigned int i = 0; i < n_vertices; i++)
  {
    Point v1 = (*elem.node_ptr((i - 1) % n_vertices) - *elem.node_ptr(i % n_vertices));
    Point v2 = (*elem.node_ptr((i + 1) % n_vertices) - *elem.node_ptr(i % n_vertices));
    Real tmp = v1 * v2 / v1.norm() / v2.norm();
    if (tmp > 1.0)
      tmp = 1.0;
    else if (tmp < -1.0)
      tmp = -1.0;
    angles.push_back(std::make_pair(acos(tmp), i));
  }
  std::sort(angles.begin(), angles.end(), std::greater<>());
  return angles;
}

std::vector<std::pair<Real, unsigned int>>
vertex_distances(const Elem & elem)
{
  std::vector<std::pair<Real, unsigned int>> distances;
  const unsigned int n_vertices = elem.n_vertices();

  for (unsigned int i = 0; i < n_vertices; i++)
  {
    Point v1 = (*elem.node_ptr((i + 1) % n_vertices) - *elem.node_ptr(i % n_vertices));
    distances.push_back(std::make_pair(v1.norm(), i));
  }
  std::sort(distances.begin(), distances.end());
  return distances;
}

void
triElemSplitter(ReplicatedMesh & mesh,
                const dof_id_type elem_id,
                const unsigned short node_shift,
                const dof_id_type nid_3,
                const dof_id_type nid_4,
                const subdomain_id_type single_elem_side_id,
                const subdomain_id_type double_elem_side_id,
                const boundary_id_type new_boundary_id)
{
  const auto elem_old = mesh.elem_ptr(elem_id);
  const dof_id_type nid_0 = elem_old->node_ptr(node_shift % 3)->id();
  const dof_id_type nid_1 = elem_old->node_ptr((1 + node_shift) % 3)->id();
  const dof_id_type nid_2 = elem_old->node_ptr((2 + node_shift) % 3)->id();

  const bool m1_side_flag =
      MooseUtils::absoluteFuzzyEqual((*(mesh.node_ptr(nid_3)) - *(mesh.node_ptr(nid_0)))
                                         .cross(*(mesh.node_ptr(nid_1)) - *(mesh.node_ptr(nid_0)))
                                         .norm(),
                                     0.0);
  const dof_id_type nid_m1 = m1_side_flag ? nid_3 : nid_4;
  const dof_id_type nid_m2 = m1_side_flag ? nid_4 : nid_3;
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(3);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[(std::get<1>(bdry_side_list[i]) + 3 - node_shift) % 3].push_back(
          std::get<2>(bdry_side_list[i]));
    }
  }

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  Elem * elem_Tri3_0 = mesh.add_elem(new Tri3);
  elem_Tri3_0->set_node(0) = mesh.node_ptr(nid_0);
  elem_Tri3_0->set_node(1) = mesh.node_ptr(nid_m1);
  elem_Tri3_0->set_node(2) = mesh.node_ptr(nid_m2);
  elem_Tri3_0->subdomain_id() = single_elem_side_id;
  Elem * elem_Tri3_1 = mesh.add_elem(new Tri3);
  elem_Tri3_1->set_node(0) = mesh.node_ptr(nid_1);
  elem_Tri3_1->set_node(1) = mesh.node_ptr(nid_m2);
  elem_Tri3_1->set_node(2) = mesh.node_ptr(nid_m1);
  elem_Tri3_1->subdomain_id() = double_elem_side_id;
  Elem * elem_Tri3_2 = mesh.add_elem(new Tri3);
  elem_Tri3_2->set_node(0) = mesh.node_ptr(nid_2);
  elem_Tri3_2->set_node(1) = mesh.node_ptr(nid_m2);
  elem_Tri3_2->set_node(2) = mesh.node_ptr(nid_1);
  elem_Tri3_2->subdomain_id() = double_elem_side_id;
  // Retain element extra integers
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
  {
    elem_Tri3_0->set_extra_integer(j, exist_extra_ids[j]);
    elem_Tri3_1->set_extra_integer(j, exist_extra_ids[j]);
    elem_Tri3_2->set_extra_integer(j, exist_extra_ids[j]);
  }

  // Add sideset information to the new elements
  for (const auto & side_info_0 : elem_side_list[0])
  {
    boundary_info.add_side(elem_Tri3_0, 0, side_info_0);
    boundary_info.add_side(elem_Tri3_1, 2, side_info_0);
  }
  for (const auto & side_info_1 : elem_side_list[1])
    boundary_info.add_side(elem_Tri3_2, 2, side_info_1);
  for (const auto & side_info_2 : elem_side_list[2])
  {
    boundary_info.add_side(elem_Tri3_0, 2, side_info_2);
    boundary_info.add_side(elem_Tri3_2, 0, side_info_2);
  }
  // Add cutting boundary to both sides as we are deleting one anyway
  boundary_info.add_side(elem_Tri3_0, 1, new_boundary_id);
  boundary_info.add_side(elem_Tri3_1, 1, new_boundary_id);
}

void
triElemSplitter(ReplicatedMesh & mesh,
                const dof_id_type elem_id,
                const unsigned short node_shift,
                const dof_id_type nid_m,
                const subdomain_id_type first_elem_side_id,
                const subdomain_id_type second_elem_side_id,
                const boundary_id_type new_boundary_id)
{
  const auto elem_old = mesh.elem_ptr(elem_id);
  const dof_id_type nid_0 = elem_old->node_ptr(node_shift % 3)->id();
  const dof_id_type nid_1 = elem_old->node_ptr((1 + node_shift) % 3)->id();
  const dof_id_type nid_2 = elem_old->node_ptr((2 + node_shift) % 3)->id();
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(3);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[(std::get<1>(bdry_side_list[i]) + 3 - node_shift) % 3].push_back(
          std::get<2>(bdry_side_list[i]));
    }
  }

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  Elem * elem_Tri3_0 = mesh.add_elem(new Tri3);
  elem_Tri3_0->set_node(0) = mesh.node_ptr(nid_0);
  elem_Tri3_0->set_node(1) = mesh.node_ptr(nid_1);
  elem_Tri3_0->set_node(2) = mesh.node_ptr(nid_m);
  elem_Tri3_0->subdomain_id() = first_elem_side_id;
  Elem * elem_Tri3_1 = mesh.add_elem(new Tri3);
  elem_Tri3_1->set_node(0) = mesh.node_ptr(nid_0);
  elem_Tri3_1->set_node(1) = mesh.node_ptr(nid_m);
  elem_Tri3_1->set_node(2) = mesh.node_ptr(nid_2);
  elem_Tri3_1->subdomain_id() = second_elem_side_id;
  // Retain element extra integers
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
  {
    elem_Tri3_0->set_extra_integer(j, exist_extra_ids[j]);
    elem_Tri3_1->set_extra_integer(j, exist_extra_ids[j]);
  }

  // Add sideset information to the new elements
  for (const auto & side_info_0 : elem_side_list[0])
    boundary_info.add_side(elem_Tri3_0, 0, side_info_0);
  for (const auto & side_info_1 : elem_side_list[1])
  {
    boundary_info.add_side(elem_Tri3_0, 1, side_info_1);
    boundary_info.add_side(elem_Tri3_1, 1, side_info_1);
  }
  for (const auto & side_info_2 : elem_side_list[2])
    boundary_info.add_side(elem_Tri3_0, 2, side_info_2);

  // Add cutting boundary to both sides as we are deleting one anyway
  boundary_info.add_side(elem_Tri3_0, 2, new_boundary_id);
  boundary_info.add_side(elem_Tri3_1, 0, new_boundary_id);
}

void
quadElemSplitter(ReplicatedMesh & mesh,
                 const dof_id_type elem_id,
                 const subdomain_id_type tri_elem_subdomain_shift)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(4);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[std::get<1>(bdry_side_list[i])].push_back(std::get<2>(bdry_side_list[i]));
    }
  }

  auto node_0 = mesh.elem_ptr(elem_id)->node_ptr(0);
  auto node_1 = mesh.elem_ptr(elem_id)->node_ptr(1);
  auto node_2 = mesh.elem_ptr(elem_id)->node_ptr(2);
  auto node_3 = mesh.elem_ptr(elem_id)->node_ptr(3);

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  // There are two trivial ways to split a quad element
  // We prefer the way that leads to triangles with similar areas
  if (std::abs((*node_1 - *node_0).cross(*node_3 - *node_0).norm() -
               (*node_1 - *node_2).cross(*node_3 - *node_2).norm()) >
      std::abs((*node_0 - *node_1).cross(*node_2 - *node_1).norm() -
               (*node_0 - *node_3).cross(*node_2 - *node_3).norm()))
  {
    Elem * elem_Tri3_0 = mesh.add_elem(new Tri3);
    elem_Tri3_0->set_node(0) = node_0;
    elem_Tri3_0->set_node(1) = node_1;
    elem_Tri3_0->set_node(2) = node_2;
    elem_Tri3_0->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + tri_elem_subdomain_shift;
    Elem * elem_Tri3_1 = mesh.add_elem(new Tri3);
    elem_Tri3_1->set_node(0) = node_0;
    elem_Tri3_1->set_node(1) = node_2;
    elem_Tri3_1->set_node(2) = node_3;
    elem_Tri3_1->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + tri_elem_subdomain_shift;
    // Retain element extra integers
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elem_Tri3_0->set_extra_integer(j, exist_extra_ids[j]);
      elem_Tri3_1->set_extra_integer(j, exist_extra_ids[j]);
    }

    // Add sideset information to the new elements
    for (const auto & side_info_0 : elem_side_list[0])
      boundary_info.add_side(elem_Tri3_0, 0, side_info_0);
    for (const auto & side_info_1 : elem_side_list[1])
      boundary_info.add_side(elem_Tri3_0, 1, side_info_1);
    for (const auto & side_info_2 : elem_side_list[2])
      boundary_info.add_side(elem_Tri3_1, 1, side_info_2);
    for (const auto & side_info_3 : elem_side_list[3])
      boundary_info.add_side(elem_Tri3_1, 2, side_info_3);
  }
  else
  {
    Elem * elem_Tri3_0 = mesh.add_elem(new Tri3);
    elem_Tri3_0->set_node(0) = node_0;
    elem_Tri3_0->set_node(1) = node_1;
    elem_Tri3_0->set_node(2) = node_3;
    elem_Tri3_0->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + tri_elem_subdomain_shift;
    Elem * elem_Tri3_1 = mesh.add_elem(new Tri3);
    elem_Tri3_1->set_node(0) = node_1;
    elem_Tri3_1->set_node(1) = node_2;
    elem_Tri3_1->set_node(2) = node_3;
    elem_Tri3_1->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + tri_elem_subdomain_shift;
    // Retain element extra integers
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elem_Tri3_0->set_extra_integer(j, exist_extra_ids[j]);
      elem_Tri3_1->set_extra_integer(j, exist_extra_ids[j]);
    }

    // Add sideset information to the new elements
    for (const auto & side_info_0 : elem_side_list[0])
      boundary_info.add_side(elem_Tri3_0, 0, side_info_0);
    for (const auto & side_info_1 : elem_side_list[1])
      boundary_info.add_side(elem_Tri3_1, 0, side_info_1);
    for (const auto & side_info_2 : elem_side_list[2])
      boundary_info.add_side(elem_Tri3_1, 1, side_info_2);
    for (const auto & side_info_3 : elem_side_list[3])
      boundary_info.add_side(elem_Tri3_0, 2, side_info_3);
  }
}

void
quadToTriOnLine(ReplicatedMesh & mesh,
                const std::vector<Real> & cut_line_params,
                const dof_id_type tri_subdomain_id_shift,
                const SubdomainName tri_elem_subdomain_name_suffix)
{
  // Preprocess: find all the quad elements that are across the cutting line
  std::vector<dof_id_type> cross_elems_quad;
  std::set<subdomain_id_type> new_subdomain_ids;
  for (auto elem_it = mesh.active_elements_begin(); elem_it != mesh.active_elements_end();
       elem_it++)
  {
    if ((*elem_it)->n_vertices() == 4)
    {
      std::vector<unsigned short> node_side_rec;
      for (unsigned int i = 0; i < 4; i++)
      {
        const Point v_point = (*elem_it)->point(i);
        node_side_rec.push_back(lineSideDeterminator(v_point(0),
                                                     v_point(1),
                                                     cut_line_params[0],
                                                     cut_line_params[1],
                                                     cut_line_params[2],
                                                     true));
      }
      if (std::accumulate(node_side_rec.begin(), node_side_rec.end(), 0) != 4 &&
          std::accumulate(node_side_rec.begin(), node_side_rec.end(), 0) > 0)
      {
        cross_elems_quad.push_back((*elem_it)->id());
        new_subdomain_ids.emplace((*elem_it)->subdomain_id() + tri_subdomain_id_shift);
      }
    }
  }
  // Then convert these quad elements into tri elements
  for (const auto & cross_elem_quad : cross_elems_quad)
  {
    quadElemSplitter(mesh, cross_elem_quad, tri_subdomain_id_shift);
    mesh.delete_elem(mesh.elem_ptr(cross_elem_quad));
  }
  for (auto & nid : new_subdomain_ids)
  {
    const SubdomainName old_name = mesh.subdomain_name(nid - tri_subdomain_id_shift);
    if (MooseMeshUtils::getSubdomainID(
            (old_name.empty() ? (SubdomainName)(std::to_string(nid - tri_subdomain_id_shift))
                              : old_name) +
                "_" + tri_elem_subdomain_name_suffix,
            mesh) != Moose::INVALID_BLOCK_ID)
      throw MooseException("The new subdomain name already exists in the mesh.");
    mesh.subdomain_name(nid) =
        (old_name.empty() ? (SubdomainName)(std::to_string(nid - tri_subdomain_id_shift))
                          : old_name) +
        "_" + tri_elem_subdomain_name_suffix;
    mooseWarning("QUAD elements have been converted into TRI elements with a new "
                 "subdomain name: " +
                 mesh.subdomain_name(nid) + ".");
  }
  mesh.contract();
}

void
lineRemoverCutElemTri(ReplicatedMesh & mesh,
                      const std::vector<Real> & cut_line_params,
                      const subdomain_id_type block_id_to_remove,
                      const boundary_id_type new_boundary_id)
{
  // Find all the elements that are across the cutting line
  std::vector<dof_id_type> cross_elems;
  // A vector for element specific information
  std::vector<std::vector<std::pair<dof_id_type, dof_id_type>>> node_pairs_vec;
  // A set for unique pairs
  std::vector<std::pair<dof_id_type, dof_id_type>> node_pairs_unique_vec;
  for (auto elem_it = mesh.active_elements_begin(); elem_it != mesh.active_elements_end();
       elem_it++)
  {
    std::vector<unsigned short> node_side_rec;
    const auto n_vertices = (*elem_it)->n_vertices();
    node_side_rec.resize(n_vertices);
    for (unsigned int i = 0; i < n_vertices; i++)
    {
      // First check if the vertex is in the XY Plane
      if (!MooseUtils::absoluteFuzzyEqual((*elem_it)->point(i)(2), 0.0))
        mooseError("MooseMeshXYCuttingUtils::lineRemoverCutElemTri() only works for 2D meshes in "
                   "XY Plane.");
      const Point v_point = (*elem_it)->point(i);
      node_side_rec[i] = lineSideDeterminator(
          v_point(0), v_point(1), cut_line_params[0], cut_line_params[1], cut_line_params[2], true);
    }
    if (std::accumulate(node_side_rec.begin(), node_side_rec.end(), 0) == (int)node_side_rec.size())
    {
      (*elem_it)->subdomain_id() = block_id_to_remove;
    }
    else if (std::accumulate(node_side_rec.begin(), node_side_rec.end(), 0) > 0)
    {
      if ((*elem_it)->n_vertices() != 3 || (*elem_it)->n_nodes() != 3)
        mooseError("The element across the cutting line is not TRI3, which is not supported.");
      cross_elems.push_back((*elem_it)->id());
      // Then we need to check pairs of nodes that are on the different side
      std::vector<std::pair<dof_id_type, dof_id_type>> node_pairs;
      for (unsigned int i = 0; i < node_side_rec.size(); i++)
      {
        // first node on removal side and second node on retaining side
        if (node_side_rec[i] > 0 && node_side_rec[(i + 1) % node_side_rec.size()] == 0)
        {
          // Removal side first
          node_pairs.push_back(
              std::make_pair((*elem_it)->node_ptr(i)->id(),
                             (*elem_it)->node_ptr((i + 1) % node_side_rec.size())->id()));
          node_pairs_unique_vec.push_back(node_pairs.back());
        }
        // first node on retaining side and second node on removal side
        else if (node_side_rec[i] == 0 && node_side_rec[(i + 1) % node_side_rec.size()] > 0)
        {
          // Removal side first
          node_pairs.push_back(
              std::make_pair((*elem_it)->node_ptr((i + 1) % node_side_rec.size())->id(),
                             (*elem_it)->node_ptr(i)->id()));
          node_pairs_unique_vec.push_back(node_pairs.back());
        }
      }
      node_pairs_vec.push_back(node_pairs);
    }
  }
  auto vec_ip = std::unique(node_pairs_unique_vec.begin(), node_pairs_unique_vec.end());
  node_pairs_unique_vec.resize(std::distance(node_pairs_unique_vec.begin(), vec_ip));

  // Loop over all the node pairs to define new nodes that sit on the cutting line
  std::vector<Node *> nodes_on_line;
  // whether the on-line node is overlapped with the node pairs or a brand new node
  std::vector<unsigned short> nodes_on_line_overlap;
  for (const auto & node_pair : node_pairs_unique_vec)
  {
    const Point pt1 = *mesh.node_ptr(node_pair.first);
    const Point pt2 = *mesh.node_ptr(node_pair.second);
    const Point pt_line = twoPointandLineIntersection(
        pt1, pt2, cut_line_params[0], cut_line_params[1], cut_line_params[2]);
    if ((pt_line - pt1).norm() < libMesh::TOLERANCE)
    {
      nodes_on_line.push_back(mesh.node_ptr(node_pair.first));
      nodes_on_line_overlap.push_back(1);
    }
    else if ((pt_line - pt2).norm() < libMesh::TOLERANCE)
    {
      nodes_on_line.push_back(mesh.node_ptr(node_pair.second));
      nodes_on_line_overlap.push_back(2);
    }
    else
    {
      nodes_on_line.push_back(mesh.add_point(pt_line));
      nodes_on_line_overlap.push_back(0);
    }
  }

  // make new elements
  for (unsigned int i = 0; i < cross_elems.size(); i++)
  {
    // Only TRI elements are involved after preprocessing
    auto cross_elem = mesh.elem_ptr(cross_elems[i]);
    auto node_0 = cross_elem->node_ptr(0);
    auto node_1 = cross_elem->node_ptr(1);
    auto node_2 = cross_elem->node_ptr(2);
    const std::vector<dof_id_type> tri_nodes = {node_0->id(), node_1->id(), node_2->id()};

    const auto online_node_index_1 = std::distance(node_pairs_unique_vec.begin(),
                                                   std::find(node_pairs_unique_vec.begin(),
                                                             node_pairs_unique_vec.end(),
                                                             node_pairs_vec[i][0]));
    const auto online_node_index_2 = std::distance(node_pairs_unique_vec.begin(),
                                                   std::find(node_pairs_unique_vec.begin(),
                                                             node_pairs_unique_vec.end(),
                                                             node_pairs_vec[i][1]));
    auto node_3 = nodes_on_line[online_node_index_1];
    auto node_4 = nodes_on_line[online_node_index_2];
    const auto node_3_overlap_flag = nodes_on_line_overlap[online_node_index_1];
    const auto node_4_overlap_flag = nodes_on_line_overlap[online_node_index_2];
    // Most common case, no overlapped nodes
    if (node_3_overlap_flag == 0 && node_4_overlap_flag == 0)
    {
      // True if the common node is on the removal side; false if on the retaining side
      const bool common_node_side = node_pairs_vec[i][0].first == node_pairs_vec[i][1].first;
      const subdomain_id_type block_id_to_assign_1 =
          common_node_side ? block_id_to_remove : cross_elem->subdomain_id();
      const subdomain_id_type block_id_to_assign_2 =
          common_node_side ? cross_elem->subdomain_id() : block_id_to_remove;
      // The reference node ids need to be adjusted according to the common node of the two cut
      // sides
      const dof_id_type common_node_id =
          common_node_side ? node_pairs_vec[i][0].first : node_pairs_vec[i][0].second;

      triElemSplitter(mesh,
                      cross_elem->id(),
                      std::distance(tri_nodes.begin(),
                                    std::find(tri_nodes.begin(), tri_nodes.end(), common_node_id)),
                      node_3->id(),
                      node_4->id(),
                      block_id_to_assign_1,
                      block_id_to_assign_2,
                      new_boundary_id);
      mesh.delete_elem(cross_elem);
    }
    // both node_3 and node_4 are overlapped
    else if (node_3_overlap_flag > 0 && node_4_overlap_flag > 0)
    {
      // In this case, the entire element is on one side of the cutting line
      // No change needed just check which side the element is on
      cross_elem->subdomain_id() = lineSideDeterminator(cross_elem->vertex_average()(0),
                                                        cross_elem->vertex_average()(1),
                                                        cut_line_params[0],
                                                        cut_line_params[1],
                                                        cut_line_params[2],
                                                        true)
                                       ? block_id_to_remove
                                       : cross_elem->subdomain_id();
    }
    // node_3 or node_4 is overlapped
    else
    {
      const auto node_3_finder = std::distance(
          tri_nodes.begin(), std::find(tri_nodes.begin(), tri_nodes.end(), node_3->id()));
      const auto node_4_finder = std::distance(
          tri_nodes.begin(), std::find(tri_nodes.begin(), tri_nodes.end(), node_4->id()));
      // As only one of the two above values should be less than the three, the smaller one should
      // be used
      const dof_id_type node_id = node_3_finder < node_4_finder ? node_4->id() : node_3->id();
      const auto node_finder = std::min(node_3_finder, node_4_finder);

      triElemSplitter(
          mesh,
          cross_elem->id(),
          node_finder,
          node_id,
          tri_nodes[(node_finder + 1) % 3] == node_pairs_vec[i][node_3_finder > node_4_finder].first
              ? block_id_to_remove
              : cross_elem->subdomain_id(),
          tri_nodes[(node_finder + 1) % 3] == node_pairs_vec[i][node_3_finder > node_4_finder].first
              ? cross_elem->subdomain_id()
              : block_id_to_remove,
          new_boundary_id);
      mesh.delete_elem(cross_elem);
    }
  }
  mesh.contract();

  // Delete the block to remove
  for (auto elem_it = mesh.active_subdomain_elements_begin(block_id_to_remove);
       elem_it != mesh.active_subdomain_elements_end(block_id_to_remove);
       elem_it++)
    mesh.delete_elem(*elem_it);
  mesh.contract();
}

void
lineRemoverCutElem(ReplicatedMesh & mesh,
                   const std::vector<Real> & cut_line_params,
                   const dof_id_type tri_subdomain_id_shift,
                   const SubdomainName tri_elem_subdomain_name_suffix,
                   const subdomain_id_type block_id_to_remove,
                   const boundary_id_type new_boundary_id,
                   const bool improve_boundary_tri_elems)
{
  // Convert any quad elements crossed by the line into tri elements
  quadToTriOnLine(mesh, cut_line_params, tri_subdomain_id_shift, tri_elem_subdomain_name_suffix);
  // Then do the cutting for the preprocessed mesh that only contains tri elements crossed by the
  // cut line
  lineRemoverCutElemTri(mesh, cut_line_params, block_id_to_remove, new_boundary_id);

  if (improve_boundary_tri_elems)
    boundaryTriElemImprover(mesh, new_boundary_id);
}

void
boundaryTriElemImprover(ReplicatedMesh & mesh, const boundary_id_type boundary_to_improve)
{
  if (!MooseMeshUtils::hasBoundaryID(mesh, boundary_to_improve))
    mooseError(
        "MooseMeshXYCuttingUtils::boundaryTriElemImprover(): The boundary_to_improve provided "
        "does not exist in the given mesh.");
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto side_list = boundary_info.build_side_list();
  // Here we would like to collect the following information for all the TRI3 elements on the
  // boundary: Key: node id of the off-boundary node Value: a vector of tuples, each tuple contains
  // the following information:
  // 1. The element id of the element that is on the boundary to improve
  // 2. the one node id of that element that is on the boundary to improve
  // 3. the other node id of the element that is on the boundary to improve
  std::map<dof_id_type, std::vector<std::tuple<dof_id_type, dof_id_type, dof_id_type>>>
      tri3_elem_info;
  for (const auto & side : side_list)
  {
    if (std::get<2>(side) == boundary_to_improve)
    {
      Elem * elem = mesh.elem_ptr(std::get<0>(side));
      if (elem->type() == TRI3)
      {
        const auto key_node_id = elem->node_id((std::get<1>(side) + 2) % 3);
        const auto value_elem_id = elem->id();
        const auto value_node_id_1 = elem->node_id(std::get<1>(side));
        const auto value_node_id_2 = elem->node_id((std::get<1>(side) + 1) % 3);
        tri3_elem_info[key_node_id].push_back(
            std::make_tuple(value_elem_id, value_node_id_1, value_node_id_2));
      }
    }
  }
  // Elements that need to be removed
  std::vector<dof_id_type> elems_to_remove;
  // Now check if any group of TRI3 sharing an off-boundary node can be improved.
  for (const auto & tri_group : tri3_elem_info)
  {
    // It is possible to improve only when more than one TRI3 elements share the same off-boundary
    // node
    std::vector<std::pair<dof_id_type, dof_id_type>> node_assm;
    std::vector<dof_id_type> elem_id_list;
    for (const auto & tri : tri_group.second)
    {
      node_assm.push_back(std::make_pair(std::get<1>(tri), std::get<2>(tri)));
      elem_id_list.push_back(std::get<0>(tri));
    }
    std::vector<dof_id_type> ordered_node_list;
    std::vector<dof_id_type> ordered_elem_list;
    MooseMeshUtils::makeOrderedNodeList(
        node_assm, elem_id_list, ordered_node_list, ordered_elem_list);

    // For all the elements sharing the same off-boundary node, we need to know how many separated
    // subdomains are involved
    // If there are extra element ids defined on the mesh, they also want to retain their boundaries
    // Only triangle elements that share a side can be merged
    const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
    std::vector<std::tuple<subdomain_id_type, std::vector<dof_id_type>, unsigned int>> blocks_info;
    for (const auto & elem_id : ordered_elem_list)
    {
      std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
      // Record all the element extra integers of the original quad element
      for (unsigned int j = 0; j < n_elem_extra_ids; j++)
        exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);
      if (!blocks_info.empty())
      {
        if (mesh.elem_ptr(elem_id)->subdomain_id() == std::get<0>(blocks_info.back()) &&
            exist_extra_ids == std::get<1>(blocks_info.back()))
        {
          std::get<2>(blocks_info.back())++;
          continue;
        }
      }
      blocks_info.push_back(
          std::make_tuple(mesh.elem_ptr(elem_id)->subdomain_id(), exist_extra_ids, 1));
    }
    // For each separated subdomain / set of extra ids, we try to improve the boundary elements
    unsigned int side_counter = 0;
    for (const auto & block_info : blocks_info)
    {
      const auto node_1 = mesh.node_ptr(ordered_node_list[side_counter]);
      // we do not need to subtract 1 for node_2
      const auto node_2 = mesh.node_ptr(ordered_node_list[side_counter + std::get<2>(block_info)]);
      const auto node_0 = mesh.node_ptr(tri_group.first);
      const Point v1 = *node_1 - *node_0;
      const Point v2 = *node_2 - *node_0;
      const Real angle = std::acos(v1 * v2 / v1.norm() / v2.norm()) / M_PI * 180.0;
      const std::vector<dof_id_type> block_elems(ordered_elem_list.begin() + side_counter,
                                                 ordered_elem_list.begin() + side_counter +
                                                     std::get<2>(block_info));
      // We assume that there are no sidesets defined inside a subdomain
      // For the first TRI3 element, we want to check if its side defined by node_0 and node_1 is
      // defined in any sidesets
      unsigned short side_id_0;
      unsigned short side_id_t;
      bool is_inverse_0;
      bool is_inverse_t;
      elemSideLocator(mesh,
                      block_elems.front(),
                      tri_group.first,
                      ordered_node_list[side_counter],
                      side_id_0,
                      is_inverse_0);
      elemSideLocator(mesh,
                      block_elems.back(),
                      ordered_node_list[side_counter + std::get<2>(block_info)],
                      tri_group.first,
                      side_id_t,
                      is_inverse_t);
      // Collect boundary information of the identified sides
      std::vector<boundary_id_type> side_0_boundary_ids;
      boundary_info.boundary_ids(
          mesh.elem_ptr(block_elems.front()), side_id_0, side_0_boundary_ids);
      std::vector<boundary_id_type> side_t_boundary_ids;
      boundary_info.boundary_ids(mesh.elem_ptr(block_elems.back()), side_id_t, side_t_boundary_ids);

      // Ideally we want this angle to be 60 degrees
      // In reality, we want one TRI3 element if the angle is less than 90 degrees;
      // we want two TRI3 elements if the angle is greater than 90 degrees and less than 135
      // degrees; we want three TRI3 elements if the angle is greater than 135 degrees and less than
      // 180 degrees.
      if (angle < 90.0)
      {
        if (std::get<2>(block_info) > 1)
        {
          makeImprovedTriElement(mesh,
                                 tri_group.first,
                                 ordered_node_list[side_counter],
                                 ordered_node_list[side_counter + std::get<2>(block_info)],
                                 std::get<0>(block_info),
                                 std::get<1>(block_info),
                                 {boundary_to_improve},
                                 side_0_boundary_ids,
                                 side_t_boundary_ids);
          elems_to_remove.insert(elems_to_remove.end(), block_elems.begin(), block_elems.end());
        }
      }
      else if (angle < 135.0)
      {
        // We can just add the middle node because there's nothing on the other side
        const auto node_m = mesh.add_point((*node_1 + *node_2) / 2.0);
        makeImprovedTriElement(mesh,
                               tri_group.first,
                               ordered_node_list[side_counter],
                               node_m->id(),
                               std::get<0>(block_info),
                               std::get<1>(block_info),
                               {boundary_to_improve},
                               side_0_boundary_ids,
                               std::vector<boundary_id_type>());
        makeImprovedTriElement(mesh,
                               tri_group.first,
                               node_m->id(),
                               ordered_node_list[side_counter + std::get<2>(block_info)],
                               std::get<0>(block_info),
                               std::get<1>(block_info),
                               {boundary_to_improve},
                               std::vector<boundary_id_type>(),
                               side_t_boundary_ids);
        elems_to_remove.insert(elems_to_remove.end(), block_elems.begin(), block_elems.end());
      }
      else
      {
        const auto node_m1 = mesh.add_point((*node_1 * 2.0 + *node_2) / 3.0);
        const auto node_m2 = mesh.add_point((*node_1 + *node_2 * 2.0) / 3.0);
        makeImprovedTriElement(mesh,
                               tri_group.first,
                               ordered_node_list[side_counter],
                               node_m1->id(),
                               std::get<0>(block_info),
                               std::get<1>(block_info),
                               {boundary_to_improve},
                               side_0_boundary_ids,
                               std::vector<boundary_id_type>());
        makeImprovedTriElement(mesh,
                               tri_group.first,
                               node_m1->id(),
                               node_m2->id(),
                               std::get<0>(block_info),
                               std::get<1>(block_info),
                               {boundary_to_improve},
                               std::vector<boundary_id_type>(),
                               std::vector<boundary_id_type>());
        makeImprovedTriElement(mesh,
                               tri_group.first,
                               node_m2->id(),
                               ordered_node_list[side_counter + std::get<2>(block_info)],
                               std::get<0>(block_info),
                               std::get<1>(block_info),
                               {boundary_to_improve},
                               std::vector<boundary_id_type>(),
                               side_t_boundary_ids);
        elems_to_remove.insert(elems_to_remove.end(), block_elems.begin(), block_elems.end());
      }
      side_counter += std::get<2>(block_info);
    }
    // TODO: Need to check if the new element is inverted?
  }
  // Delete the original elements
  for (const auto & elem_to_remove : elems_to_remove)
    mesh.delete_elem(mesh.elem_ptr(elem_to_remove));
  mesh.contract();
}

void
makeImprovedTriElement(ReplicatedMesh & mesh,
                       const dof_id_type node_id_0,
                       const dof_id_type node_id_1,
                       const dof_id_type node_id_2,
                       const subdomain_id_type subdomain_id,
                       const std::vector<dof_id_type> & extra_elem_ids,
                       const std::vector<boundary_id_type> & boundary_ids_for_side_1,
                       const std::vector<boundary_id_type> & boundary_ids_for_side_0,
                       const std::vector<boundary_id_type> & boundary_ids_for_side_2)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  Elem * elem_Tri3_new = mesh.add_elem(new Tri3);
  elem_Tri3_new->set_node(0) = mesh.node_ptr(node_id_0);
  elem_Tri3_new->set_node(1) = mesh.node_ptr(node_id_1);
  elem_Tri3_new->set_node(2) = mesh.node_ptr(node_id_2);
  for (const auto & boundary_id_for_side_0 : boundary_ids_for_side_0)
    boundary_info.add_side(elem_Tri3_new, 0, boundary_id_for_side_0);
  for (const auto & boundary_id_for_side_1 : boundary_ids_for_side_1)
    boundary_info.add_side(elem_Tri3_new, 1, boundary_id_for_side_1);
  for (const auto & boundary_id_for_side_2 : boundary_ids_for_side_2)
    boundary_info.add_side(elem_Tri3_new, 2, boundary_id_for_side_2);
  elem_Tri3_new->subdomain_id() = subdomain_id;
  // Retain element extra integers
  for (unsigned int j = 0; j < extra_elem_ids.size(); j++)
  {
    elem_Tri3_new->set_extra_integer(j, extra_elem_ids[j]);
  }
}

bool
elemSideLocator(ReplicatedMesh & mesh,
                const dof_id_type elem_id,
                const dof_id_type node_id_0,
                const dof_id_type node_id_1,
                unsigned short & side_id,
                bool & is_inverse)
{
  Elem * elem = mesh.elem_ptr(elem_id);
  for (unsigned short i = 0; i < elem->n_sides(); i++)
  {
    if (elem->side_ptr(i)->node_ptr(0)->id() == node_id_0 &&
        elem->side_ptr(i)->node_ptr(1)->id() == node_id_1)
    {
      side_id = i;
      is_inverse = false;
      return true;
    }
    else if (elem->side_ptr(i)->node_ptr(0)->id() == node_id_1 &&
             elem->side_ptr(i)->node_ptr(1)->id() == node_id_0)
    {
      side_id = i;
      is_inverse = true;
      return true;
    }
  }
  return false;
}
}
