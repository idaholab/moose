//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriToQuadGenerator.h"

#include "MooseMeshElementConversionUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/int_range.h"
#include "libmesh/libmesh.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/string_to_enum.h"

// C++ includes
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <unordered_set>

registerMooseObject("MooseApp", TriToQuadGenerator);

// The mesh generator sources are compiled as one unity translation unit, which puts this file scope
// and that of every sibling generator together, so no name here may collide with a sibling's
namespace
{
/// Number of sides of a TRI3 element
constexpr unsigned int n_tri_sides = 3;

/// Number of sides of a QUAD4 element
constexpr unsigned int n_quad_sides = 4;

/**
 * Twice the signed area of a triangle in the xy plane, which is positive when its vertices are
 * ordered counter-clockwise.
 * @param point_1 the first vertex of the triangle
 * @param point_2 the second vertex of the triangle
 * @param point_3 the third vertex of the triangle
 * @return twice the signed area
 */
Real
signedArea2D(const Point & point_1, const Point & point_2, const Point & point_3)
{
  return (point_2(0) - point_1(0)) * (point_3(1) - point_1(1)) -
         (point_2(1) - point_1(1)) * (point_3(0) - point_1(0));
}

/**
 * The key that identifies an edge of the mesh, which is the ids of its two end nodes in increasing
 * order.
 * @param node_id_1 the id of the node at one end of the edge
 * @param node_id_2 the id of the node at the other end of the edge
 * @return the edge key
 */
std::pair<dof_id_type, dof_id_type>
edgeKey(const dof_id_type node_id_1, const dof_id_type node_id_2)
{
  return std::make_pair(std::min(node_id_1, node_id_2), std::max(node_id_1, node_id_2));
}

/**
 * The node at the midpoint of the edge between two nodes, created on the first request for that
 * edge and reused afterwards.
 * @param mesh the mesh that owns the nodes
 * @param midpoints the midpoints created so far, keyed on the edge that they split
 * @param node_1 the node at one end of the edge
 * @param node_2 the node at the other end of the edge
 * @return the midpoint node
 */
Node *
edgeMidpointNode(MeshBase & mesh,
                 std::map<std::pair<dof_id_type, dof_id_type>, Node *> & midpoints,
                 const Node & node_1,
                 const Node & node_2)
{
  const auto edge = edgeKey(node_1.id(), node_2.id());
  const auto it = midpoints.find(edge);
  if (it != midpoints.end())
    return it->second;

  Node * const midpoint = mesh.add_point((node_1 + node_2) / 2.0);
  midpoints.emplace(edge, midpoint);

  return midpoint;
}

/**
 * The quadrilaterals that an element with the given number of corners is split into, one per
 * corner. Each of them is built on that corner, the midpoints of the two sides the corner is an end
 * of, and the centroid.
 * @param n_corners the number of corners of the element
 * @return the quadrilaterals, as indices into a point list holding the corners first, then the
 * midpoint of the side running from corner s to corner s + 1, then the centroid
 */
std::vector<TriToQuadGenerator::QuadCorners>
subdivisionTemplate(const unsigned int n_corners)
{
  std::vector<TriToQuadGenerator::QuadCorners> quads;
  for (const auto k : make_range(n_corners))
    quads.push_back({k, n_corners + k, 2 * n_corners, n_corners + (k + n_corners - 1) % n_corners});

  return quads;
}

/**
 * The index of the side of a quadrilateral whose two end nodes are the given nodes.
 * @param quad_node_ids the node ids of the quadrilateral, in order
 * @param node_id_1 the id of the node at one end of the edge
 * @param node_id_2 the id of the node at the other end of the edge
 * @return the side index, or libMesh::invalid_uint if the quadrilateral has no such side
 */
unsigned int
quadSideOnEdge(const std::array<dof_id_type, 4> & quad_node_ids,
               const dof_id_type node_id_1,
               const dof_id_type node_id_2)
{
  for (const auto s : index_range(quad_node_ids))
  {
    const dof_id_type end_1 = quad_node_ids[s];
    const dof_id_type end_2 = quad_node_ids[(s + 1) % quad_node_ids.size()];
    if ((end_1 == node_id_1 && end_2 == node_id_2) || (end_1 == node_id_2 && end_2 == node_id_1))
      return cast_int<unsigned int>(s);
  }

  return libMesh::invalid_uint;
}

/**
 * Delete the elements parked in the scratch subdomain, compact the mesh and bring its boundary
 * information back in sync.
 * @param mesh the mesh being converted
 * @param scratch_subdomain_id the subdomain holding the elements that the conversion consumed
 */
void
removeScratchElements(MeshBase & mesh, const subdomain_id_type scratch_subdomain_id)
{
  for (auto elem_it = mesh.active_subdomain_elements_begin(scratch_subdomain_id);
       elem_it != mesh.active_subdomain_elements_end(scratch_subdomain_id);
       ++elem_it)
    mesh.delete_elem(*elem_it);

  mesh.contract();
  mesh.prepare_for_use();

  // build_node_list_from_side_list() only adds nodes, so the node sets of the input mesh survive
  // and the nodes created on a boundary edge join the node set of that boundary
  mesh.get_boundary_info().build_node_list_from_side_list();
}
}

InputParameters
TriToQuadGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input",
                                             "The TRI3 mesh to convert into QUAD4 elements.");

  MooseEnum algorithm("SUBDIVISION RECOMBINE", "RECOMBINE");
  params.addParam<MooseEnum>("algorithm",
                             algorithm,
                             "The algorithm used to build the quadrilaterals. 'SUBDIVISION' splits "
                             "every triangle into three quadrilaterals. 'RECOMBINE' merges pairs "
                             "of adjacent triangles into quadrilaterals.");

  MooseEnum matching("GREEDY", "GREEDY");
  params.addParam<MooseEnum>(
      "matching",
      matching,
      "The algorithm used to pair adjacent triangles with the 'RECOMBINE' algorithm.");

  params.addRangeCheckedParam<Real>(
      "eta_min",
      0.3,
      "eta_min > 0 & eta_min <= 1",
      "'RECOMBINE' algorithm only: the quality score that a pair of adjacent triangles must reach "
      "to be merged into a quadrilateral. A perfect rectangle scores 1.");

  params.addParam<SubdomainName>(
      "tri_subdomain_name",
      "'RECOMBINE' algorithm only: the name of the subdomain that the triangles which could not be "
      "merged are moved into. Those triangles stay in their original subdomain if this parameter "
      "is not set.");

  params.addParam<bool>("all_quad",
                        false,
                        "'RECOMBINE' algorithm only: whether the triangles that could not be "
                        "merged are eliminated so that the converted mesh consists exclusively of "
                        "quadrilaterals.");

  params.addParamNamesToGroup("matching eta_min tri_subdomain_name all_quad", "Recombination");

  params.addClassDescription("Converts a mesh consisting of TRI3 elements into a mesh consisting "
                             "of QUAD4 elements, either by splitting every triangle into three "
                             "quadrilaterals or by merging pairs of adjacent triangles.");

  return params;
}

TriToQuadGenerator::TriToQuadGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _algorithm(getParam<MooseEnum>("algorithm")),
    _matching(getParam<MooseEnum>("matching")),
    _eta_min(getParam<Real>("eta_min")),
    _has_tri_subdomain(isParamValid("tri_subdomain_name")),
    _tri_subdomain_name(_has_tri_subdomain ? getParam<SubdomainName>("tri_subdomain_name")
                                           : SubdomainName()),
    _all_quad(getParam<bool>("all_quad"))
{
  if (_all_quad && _algorithm == "SUBDIVISION")
    paramError("all_quad",
               "The 'all_quad' option is only available with the 'RECOMBINE' algorithm.");

  if (_all_quad && _has_tri_subdomain)
    paramError("all_quad",
               "The 'all_quad' option leaves no triangle for 'tri_subdomain_name' to name.");
}

std::unique_ptr<MeshBase>
TriToQuadGenerator::generate()
{
  auto replicated_mesh_ptr = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!replicated_mesh_ptr)
    paramError("input", "Input is not a replicated mesh, which is required");

  ReplicatedMesh & mesh = *replicated_mesh_ptr;

  // The conversion is driven by the neighbor links, and those are only built, along with the
  // element ranges that the loops below rely on, once the mesh is prepared
  if (!mesh.is_prepared())
    mesh.prepare_for_use();

  for (const auto & elem : mesh.element_ptr_range())
    if (elem->type() != libMesh::ElemType::TRI3)
      paramError("input",
                 "Element ",
                 elem->id(),
                 " is a ",
                 libMesh::Utility::enum_to_string(elem->type()),
                 " element. Only meshes consisting exclusively of TRI3 elements are supported.");

  // Both algorithms measure the areas and the angles that decide the conversion in the XY plane
  for (const auto & node : mesh.node_ptr_range())
    if ((*node)(2) != 0.0)
      paramError("input",
                 "Node ",
                 node->id(),
                 " is at z = ",
                 (*node)(2),
                 ". Only meshes in the XY plane are supported.");

  if (_algorithm == "SUBDIVISION")
    subdivide(mesh);
  else
    recombine(mesh);

  return std::move(_input);
}

Real
TriToQuadGenerator::quadQuality(const std::array<Point, 4> & quad_points)
{
  Real max_deviation = 0.0;
  for (const auto k : index_range(quad_points))
  {
    const Point incoming = quad_points[k] - quad_points[(k + 3) % quad_points.size()];
    const Point outgoing = quad_points[(k + 1) % quad_points.size()] - quad_points[k];

    // For a counter-clockwise ordering the turn from the incoming to the outgoing edge is positive
    // at a convex corner, so a non-positive turn is what makes an internal angle reach pi
    const Real turn = std::atan2(incoming(0) * outgoing(1) - incoming(1) * outgoing(0),
                                 incoming(0) * outgoing(0) + incoming(1) * outgoing(1));
    const Real alpha = libMesh::pi - turn;
    if (alpha >= libMesh::pi)
      return 0.0;

    max_deviation = std::max(max_deviation, std::abs(libMesh::pi / 2.0 - alpha));
  }

  return std::max(0.0, 1.0 - 2.0 / libMesh::pi * max_deviation);
}

std::vector<TriToQuadGenerator::RecombineCandidate>
TriToQuadGenerator::greedyMatching(std::vector<RecombineCandidate> & candidates, const Real eta_min)
{
  // Ties are broken by element id so that the same pairs are merged from one run to the next
  std::sort(candidates.begin(),
            candidates.end(),
            [](const RecombineCandidate & a, const RecombineCandidate & b)
            {
              if (a.eta > b.eta)
                return true;
              if (b.eta > a.eta)
                return false;
              if (a.first_elem_id != b.first_elem_id)
                return a.first_elem_id < b.first_elem_id;
              return a.second_elem_id < b.second_elem_id;
            });

  std::vector<RecombineCandidate> selected;
  std::unordered_set<dof_id_type> consumed;
  for (const auto & candidate : candidates)
  {
    // The candidates are sorted by decreasing score, so nothing below the threshold is left
    if (candidate.eta < eta_min)
      break;
    if (consumed.count(candidate.first_elem_id) || consumed.count(candidate.second_elem_id))
      continue;

    consumed.insert(candidate.first_elem_id);
    consumed.insert(candidate.second_elem_id);
    selected.push_back(candidate);
  }

  return selected;
}

std::vector<TriToQuadGenerator::QuadCorners>
TriToQuadGenerator::triSubdivisionTemplate()
{
  return subdivisionTemplate(n_tri_sides);
}

std::vector<TriToQuadGenerator::QuadCorners>
TriToQuadGenerator::quadSubdivisionTemplate()
{
  return subdivisionTemplate(n_quad_sides);
}

TriToQuadGenerator::RecombineCandidate
TriToQuadGenerator::buildCandidate(const Elem & elem,
                                   const unsigned int side,
                                   const Elem & neighbor)
{
  // The quadrilateral runs from the apex of one triangle, along the shared edge, to the apex of
  // the other triangle and back along the shared edge
  const unsigned int neighbor_side = neighbor.which_neighbor_am_i(&elem);
  std::array<const Node *, 4> corners = {elem.node_ptr((side + 2) % n_tri_sides),
                                         elem.node_ptr(side),
                                         neighbor.node_ptr((neighbor_side + 2) % n_tri_sides),
                                         elem.node_ptr((side + 1) % n_tri_sides)};

  // Order the corners counter-clockwise so that the quadrilateral has a positive Jacobian
  if (signedArea2D(*corners[0], *corners[1], *corners[2]) +
          signedArea2D(*corners[0], *corners[2], *corners[3]) <
      0.0)
    std::reverse(corners.begin(), corners.end());

  const std::array<Point, 4> quad_points = {*corners[0], *corners[1], *corners[2], *corners[3]};

  RecombineCandidate candidate;
  candidate.eta = quadQuality(quad_points);
  candidate.first_elem_id = std::min(elem.id(), neighbor.id());
  candidate.second_elem_id = std::max(elem.id(), neighbor.id());
  for (const auto k : index_range(corners))
    candidate.quad_node_ids[k] = corners[k]->id();

  return candidate;
}

void
TriToQuadGenerator::subdivide(ReplicatedMesh & mesh) const
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  const auto bdry_side_list = boundary_info.build_side_list();

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  const auto scratch_subdomain_id = MooseMeshUtils::getNextFreeSubdomainID(mesh);

  // Keying the midpoints on the sorted node id pair of their edge makes the two elements sharing
  // that edge use the same node, which is what keeps the converted mesh conformal
  std::map<std::pair<dof_id_type, dof_id_type>, Node *> edge_midpoints;

  // The elements are collected up front because the loop below adds elements to the mesh
  std::vector<dof_id_type> elem_ids;
  for (const auto & elem : mesh.active_element_ptr_range())
    elem_ids.push_back(elem->id());

  for (const auto elem_id : elem_ids)
  {
    Elem * const parent = mesh.elem_ptr(elem_id);
    const unsigned int n_corners = parent->n_nodes();

    std::vector<std::vector<boundary_id_type>> elem_side_list;
    MooseMeshElementConversionUtils::elementBoundaryInfoCollector(
        bdry_side_list, elem_id, cast_int<unsigned short>(n_corners), elem_side_list);

    // Walking the vertices counter-clockwise gives the quadrilaterals a positive Jacobian
    // whichever way the element itself is oriented. Every element here is convex, so the turn at
    // its first corner is the turn of the whole element
    const bool clockwise =
        signedArea2D(*parent->node_ptr(0), *parent->node_ptr(1), *parent->node_ptr(2)) < 0.0;

    std::vector<unsigned int> vertex_index(n_corners);
    std::vector<unsigned int> side_index(n_corners);
    for (const auto k : make_range(n_corners))
    {
      vertex_index[k] = clockwise ? (n_corners - k) % n_corners : k;
      side_index[k] = clockwise ? (2 * n_corners - k - 1) % n_corners : k;
    }

    std::vector<Node *> vertices(n_corners);
    for (const auto k : index_range(vertices))
      vertices[k] = parent->node_ptr(vertex_index[k]);

    std::vector<Node *> midpoints(n_corners);
    for (const auto k : index_range(midpoints))
    {
      const Node & next_vertex = *vertices[(k + 1) % vertices.size()];
      midpoints[k] = edgeMidpointNode(mesh, edge_midpoints, *vertices[k], next_vertex);
    }

    // Unlike the edge midpoints, the vertex average is not shared with any other element
    Node * const centroid = mesh.add_point(parent->vertex_average());

    // The point list the template indexes into holds the corners, then the midpoints, then the
    // centroid
    std::vector<Node *> points = vertices;
    points.insert(points.end(), midpoints.begin(), midpoints.end());
    points.push_back(centroid);

    const auto quad_template = subdivisionTemplate(n_corners);

    std::vector<Elem *> quads;
    for (const auto & quad_corners : quad_template)
    {
      Elem * const quad = mesh.add_elem(Elem::build(libMesh::ElemType::QUAD4));
      for (const auto k : index_range(quad_corners))
        quad->set_node(k, points[quad_corners[k]]);
      quad->subdomain_id() = parent->subdomain_id();
      for (const auto j : make_range(n_elem_extra_ids))
        quad->set_extra_integer(j, parent->get_extra_integer(j));

      quads.push_back(quad);
    }

    // Each side of the element is split between two of the quadrilaterals, which both inherit the
    // boundary ids that the side carried
    for (const auto k : index_range(quads))
      for (const auto bid : elem_side_list[side_index[k]])
      {
        boundary_info.add_side(quads[k], 0, bid);
        boundary_info.add_side(quads[(k + 1) % quads.size()], 3, bid);
      }

    parent->subdomain_id() = scratch_subdomain_id;
  }

  removeScratchElements(mesh, scratch_subdomain_id);
}

void
TriToQuadGenerator::recombine(ReplicatedMesh & mesh) const
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  const auto bdry_side_list = boundary_info.build_side_list();

  // Keyed by edge rather than by element side so that an id registered on either of the two sides
  // of an interior edge is found
  std::set<std::pair<dof_id_type, dof_id_type>> boundary_edges;
  for (const auto & [elem_id, side, _] : bdry_side_list)
  {
    const Elem & elem = *mesh.elem_ptr(elem_id);
    boundary_edges.insert(edgeKey(elem.node_id(side), elem.node_id((side + 1) % n_tri_sides)));
  }

  std::vector<RecombineCandidate> candidates;
  for (const auto & elem : mesh.active_element_ptr_range())
    for (const auto s : make_range(elem->n_sides()))
    {
      const Elem * const neighbor = elem->neighbor_ptr(s);
      // An edge on the exterior boundary has nothing to merge with, and merging across a block
      // interface would move that interface
      if (!neighbor || neighbor->subdomain_id() != elem->subdomain_id())
        continue;
      // Each interior edge is visited from its lower numbered element only
      if (neighbor->id() < elem->id())
        continue;
      // Merging deletes the shared edge, which would take any sideset on it with it, and the
      // conversion has to preserve every sideset of the input mesh
      if (boundary_edges.count(edgeKey(elem->node_id(s), elem->node_id((s + 1) % n_tri_sides))))
        continue;

      candidates.push_back(buildCandidate(*elem, s, *neighbor));
    }

  mooseAssert(_matching == "GREEDY", "GREEDY is the only option of the matching parameter.");
  const auto merges = greedyMatching(candidates, _eta_min);

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  const auto scratch_subdomain_id = MooseMeshUtils::getNextFreeSubdomainID(mesh);

  for (const auto & merge : merges)
  {
    Elem * const first = mesh.elem_ptr(merge.first_elem_id);
    Elem * const second = mesh.elem_ptr(merge.second_elem_id);

    Elem * const quad = mesh.add_elem(Elem::build(libMesh::ElemType::QUAD4));
    for (const auto k : index_range(merge.quad_node_ids))
      quad->set_node(k, mesh.node_ptr(merge.quad_node_ids[k]));
    quad->subdomain_id() = first->subdomain_id();

    // The two triangles may carry different extra integers, so they always come from the lower
    // numbered one to keep the result reproducible
    for (const auto j : make_range(n_elem_extra_ids))
      quad->set_extra_integer(j, first->get_extra_integer(j));

    for (const Elem * const parent : {first, second})
    {
      std::vector<std::vector<boundary_id_type>> elem_side_list;
      MooseMeshElementConversionUtils::elementBoundaryInfoCollector(
          bdry_side_list, parent->id(), cast_int<unsigned short>(n_tri_sides), elem_side_list);

      for (const auto s : index_range(elem_side_list))
      {
        if (elem_side_list[s].empty())
          continue;

        const auto quad_side = quadSideOnEdge(
            merge.quad_node_ids, parent->node_id(s), parent->node_id((s + 1) % n_tri_sides));
        mooseAssert(quad_side != libMesh::invalid_uint,
                    "A parent side carrying boundary ids must be a side of the merged "
                    "quadrilateral, because pairs whose shared edge carries any boundary id are "
                    "never merged.");

        for (const auto bid : elem_side_list[s])
          boundary_info.add_side(quad, quad_side, bid);
      }
    }

    first->subdomain_id() = scratch_subdomain_id;
    second->subdomain_id() = scratch_subdomain_id;
  }

  if (_has_tri_subdomain)
    moveSurvivingTriangles(mesh, scratch_subdomain_id);

  removeScratchElements(mesh, scratch_subdomain_id);

  // Splitting every element, rather than only the triangles, is what keeps the mesh conformal: a
  // triangle split on its own would leave the elements next to it with a node in the middle of a
  // side. The merges come first so that the pairs that did merge only split into four
  if (_all_quad)
    subdivide(mesh);
}

void
TriToQuadGenerator::moveSurvivingTriangles(ReplicatedMesh & mesh,
                                           const subdomain_id_type scratch_subdomain_id) const
{
  // The scratch subdomain is empty when nothing was merged, so its id is taken as the reference
  // rather than asking the mesh for its highest subdomain id again
  const subdomain_id_type tri_subdomain_id = scratch_subdomain_id + 1;

  bool has_surviving_tri = false;
  for (const auto & elem : mesh.active_element_ptr_range())
    if (elem->type() == libMesh::ElemType::TRI3 && elem->subdomain_id() != scratch_subdomain_id)
    {
      elem->subdomain_id() = tri_subdomain_id;
      has_surviving_tri = true;
    }

  // Naming a subdomain that holds no element would leave the mesh with a block name that matches
  // nothing
  if (has_surviving_tri)
    mesh.subdomain_name(tri_subdomain_id) = _tri_subdomain_name;
}
