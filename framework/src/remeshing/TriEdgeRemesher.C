//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriEdgeRemesher.h"

#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"

#include <algorithm>
#include <cmath>

using namespace libMesh;

namespace
{
/**
 * The metric length above which an edge is split and the length below which it is collapsed. The
 * classical sqrt(2) pair keeps the two operations out of each other's output: splitting halves a
 * length that was above sqrt(2), which lands the children above 1/sqrt(2), so a split never feeds
 * the collapse pass, and a collapse that would create an edge above the split bound is refused.
 */
const Real split_bound = std::sqrt(2.0);
const Real collapse_bound = 1.0 / std::sqrt(2.0);

/**
 * The shape quality below which a collapse may not leave a triangle, unless the fan it replaces
 * was already worse, in which case not degrading it is enough. 0.3 sits well under the 0.866 a
 * structured or red-green triangle scores, so ordinary coarsening is never refused for shape, and
 * well above zero, so a collapse cannot leave a sliver behind in a healthy fan.
 */
const Real collapse_quality_floor = 0.3;

/**
 * How much a swap has to improve the worse shape quality of its two triangles to be accepted. A
 * strict improvement is what makes the swap pass a descent rather than a cycle, and 1e-6 on a
 * quality in [0, 1] refuses only the swaps whose gain is round-off.
 */
const Real swap_improvement = 1e-6;
}

registerMooseObject("MooseApp", TriEdgeRemesher);

InputParameters
TriEdgeRemesher::validParams()
{
  InputParameters params = Remesher::validParams();
  params.addClassDescription(
      "Adapts a triangle mesh toward a target element size field by local edge operations, "
      "splitting the edges that are long against the target, collapsing the ones that are short, "
      "and swapping the ones whose swap improves element shape.");

  params.addRequiredParam<VariableName>(
      "sizing_variable",
      "Target element size field, as a CONSTANT MONOMIAL variable that carries one target size per "
      "element. An edge longer than sqrt(2) times the target interpolated to it is split, and an "
      "edge shorter than 1/sqrt(2) times it is collapsed.");
  params.addRangeCheckedParam<Real>(
      "min_element_size",
      "min_element_size > 0",
      "Floor the target element size is held at: where the sizing variable asks for less, the "
      "floor takes over, and the variable may then fall to zero or below there. When this is not "
      "set the sizing variable itself must be positive on every element.");
  params.addRangeCheckedParam<unsigned int>(
      "max_iterations",
      3,
      "max_iterations > 0",
      "Number of split-collapse-swap rounds one remesh event performs. A round leaves at most a "
      "factor of two in edge length, so a few rounds absorb a target that changed by more than "
      "that between events, and an event stops early once a round changes nothing.");

  return params;
}

TriEdgeRemesher::TriEdgeRemesher(const InputParameters & parameters)
  : Remesher(parameters),
    _sizing_variable(_fe_problem.getVariable(/*tid=*/0,
                                             getParam<VariableName>("sizing_variable"),
                                             Moose::VarKindType::VAR_ANY,
                                             Moose::VarFieldType::VAR_FIELD_STANDARD)),
    _min_element_size(isParamValid("min_element_size")
                          ? std::make_optional(getParam<Real>("min_element_size"))
                          : std::nullopt),
    _max_iterations(getParam<unsigned int>("max_iterations"))
{
  if (_mesh.dimension() != 2)
    mooseError("This remesher adapts triangles in the XY plane, but the mesh is ",
               _mesh.dimension(),
               "D.");

  if (_mesh.isDistributedMesh())
    mooseError("This remesher performs the identical surgery on every rank of a replicated mesh, "
               "and does not yet confine its operations to the elements a rank owns, so it cannot "
               "run on a distributed mesh.");

  checkElementalSizingVariable("sizing_variable", _sizing_variable);

  for (const Elem * elem : _mesh.getMesh().active_element_ptr_range())
    if (elem->type() != TRI3)
      mooseError("This remesher adapts TRI3 elements, but the mesh also contains ",
                 Utility::enum_to_string(elem->type()),
                 " elements. Build the mesh out of triangles to remesh it this way.");
}

Remesher::RemeshRecord
TriEdgeRemesher::remesh()
{
  RemeshRecord record;

  const std::map<dof_id_type, Real> target_sizes = buildTargetSizeTable();
  ShadowMesh shadow = buildShadowMesh(target_sizes);

  unsigned int n_splits = 0;
  unsigned int n_collapses = 0;
  unsigned int n_swaps = 0;
  for (const auto iteration : make_range(_max_iterations))
  {
    libmesh_ignore(iteration);
    const unsigned int splits = splitPass(shadow);
    const unsigned int collapses = collapsePass(shadow);
    const unsigned int swaps = swapPass(shadow);
    n_splits += splits;
    n_collapses += collapses;
    n_swaps += swaps;
    if (splits + collapses + swaps == 0)
      break;
  }

  if (n_splits + n_collapses + n_swaps > 0)
    _console << "Remeshing: " << n_splits << " edge splits, " << n_collapses << " edge collapses, "
             << n_swaps << " edge swaps" << std::endl;

  spliceShadow(shadow, record);

  record.changed = !record.new_elements.empty();
  return record;
}

std::map<dof_id_type, Real>
TriEdgeRemesher::buildTargetSizeTable() const
{
  const MeshBase & mesh = _mesh.getMesh();

  // The ids and the targets of the owned elements that carry one, as two parallel vectors because
  // that is what the communicator gathers
  std::vector<dof_id_type> target_ids;
  std::vector<Real> targets;

  for (const Elem * elem : mesh.active_element_ptr_range())
  {
    // Only the owner of an element can read the target on it: current_local_solution is readable
    // at the owned degrees of freedom plus the send list, and the degrees of freedom partition by
    // processor id even where the elements do not
    if (elem->processor_id() != mesh.processor_id())
      continue;

    const std::optional<Real> target = readTargetSize(_sizing_variable, *elem, _min_element_size);
    if (!target)
      continue;

    target_ids.push_back(elem->id());
    targets.push_back(*target);
  }

  // Every rank performs the whole surgery, so the readings narrowed to the owned elements are
  // gathered for every rank to size every edge the same way
  return gatherTargetSizes(std::move(target_ids), std::move(targets));
}

TriEdgeRemesher::ShadowMesh
TriEdgeRemesher::buildShadowMesh(const std::map<dof_id_type, Real> & target_sizes) const
{
  MeshBase & mesh = _mesh.getMesh();
  const BoundaryInfo & boundary_info = mesh.get_boundary_info();

  ShadowMesh shadow;

  // One vertex per node in increasing node id order, so that every rank builds the same shadow
  std::vector<dof_id_type> node_ids;
  for (const Node * node : mesh.node_ptr_range())
    node_ids.push_back(node->id());
  std::sort(node_ids.begin(), node_ids.end());

  std::map<dof_id_type, std::size_t> node_vertex;
  for (const auto node_id : node_ids)
  {
    Node * node = mesh.node_ptr(node_id);
    node_vertex.emplace(node_id, shadow.vertices.size());
    ShadowVertex vertex;
    vertex.node = node;
    vertex.point = *node;
    shadow.vertices.push_back(vertex);
    shadow.vertex_triangles.emplace_back();
  }

  // One triangle per element in increasing element id order
  std::vector<dof_id_type> element_ids;
  for (const Elem * elem : mesh.active_element_ptr_range())
    element_ids.push_back(elem->id());
  std::sort(element_ids.begin(), element_ids.end());

  std::vector<boundary_id_type> side_ids;
  for (const auto elem_id : element_ids)
  {
    const Elem & elem = *mesh.elem_ptr(elem_id);

    ShadowTriangle triangle;
    for (const auto n : make_range(3u))
      triangle.vertices[n] = libmesh_map_find(node_vertex, elem.node_id(n));
    triangle.subdomain = elem.subdomain_id();
    triangle.original_id = elem_id;
    triangle.ancestors = {elem_id};

    const std::size_t t = shadow.triangles.size();
    shadow.triangles.push_back(triangle);
    for (const auto n : make_range(3u))
    {
      const EdgeKey key = edgeKey(triangle.vertices[n], triangle.vertices[(n + 1) % 3]);
      shadow.edge_triangles[key].push_back(t);
      shadow.vertex_triangles[triangle.vertices[n]].insert(t);
    }

    // The boundary ids the sides carry, keyed by the vertex pair so that splits can hand them on
    for (const auto side : elem.side_index_range())
    {
      boundary_info.boundary_ids(&elem, side, side_ids);
      if (side_ids.empty())
        continue;

      const auto side_nodes = elem.nodes_on_side(side);
      const EdgeKey key = edgeKey(libmesh_map_find(node_vertex, elem.node_id(side_nodes[0])),
                                  libmesh_map_find(node_vertex, elem.node_id(side_nodes[1])));
      shadow.edge_boundary_ids[key] = side_ids;
    }

    // The target at a vertex is the smallest target of the elements around it, which is the
    // conservative reading: an edge is measured against the finest size any element on it asks for
    const auto it = target_sizes.find(elem_id);
    if (it != target_sizes.end())
      for (const auto n : make_range(3u))
      {
        std::optional<Real> & target = shadow.vertices[triangle.vertices[n]].target;
        target = target ? std::min(*target, it->second) : it->second;
      }
  }

  // A vertex on an exterior boundary, on a sideset or on a subdomain seam is locked: a collapse
  // may never remove it, so those features pass through the surgery unchanged
  for (const auto & [key, tris] : shadow.edge_triangles)
    if (tris.size() == 1 || shadow.edge_boundary_ids.count(key) ||
        shadow.triangles[tris[0]].subdomain != shadow.triangles[tris[1]].subdomain)
    {
      shadow.vertices[key.first].locked = true;
      shadow.vertices[key.second].locked = true;
    }

  // A node carrying nodeset ids is a feature of its own even where no side does
  for (const auto & [node_id, vertex] : node_vertex)
  {
    boundary_info.boundary_ids(mesh.node_ptr(node_id), side_ids);
    if (!side_ids.empty())
      shadow.vertices[vertex].locked = true;
  }

  return shadow;
}

std::optional<Real>
TriEdgeRemesher::metricLength(const ShadowMesh & shadow, const std::size_t a, const std::size_t b)
{
  const std::optional<Real> & ta = shadow.vertices[a].target;
  const std::optional<Real> & tb = shadow.vertices[b].target;
  if (!ta && !tb)
    return std::nullopt;

  const Real target = ta && tb ? (*ta + *tb) / 2.0 : (ta ? *ta : *tb);
  return (shadow.vertices[a].point - shadow.vertices[b].point).norm() / target;
}

bool
TriEdgeRemesher::edgeConstrained(const ShadowMesh & shadow, const EdgeKey & key)
{
  const std::vector<std::size_t> & tris = libmesh_map_find(shadow.edge_triangles, key);
  return tris.size() == 1 || shadow.edge_boundary_ids.count(key) ||
         shadow.triangles[tris[0]].subdomain != shadow.triangles[tris[1]].subdomain;
}

Real
TriEdgeRemesher::signedArea(const Point & a, const Point & b, const Point & c)
{
  return ((b(0) - a(0)) * (c(1) - a(1)) - (b(1) - a(1)) * (c(0) - a(0))) / 2.0;
}

Real
TriEdgeRemesher::shapeQuality(const Point & a, const Point & b, const Point & c)
{
  const Real sum_sq = (b - a).norm_sq() + (c - b).norm_sq() + (a - c).norm_sq();
  if (sum_sq == 0)
    return 0;

  // 4*sqrt(3)*A / (l0^2 + l1^2 + l2^2) is 1 for an equilateral triangle and 0 for a degenerate
  // one. The area is signed, so a triangle wound clockwise scores negative and fails any positive
  // bound, which is what lets one test refuse inversion and slivers together.
  return 4.0 * std::sqrt(3.0) * signedArea(a, b, c) / sum_sq;
}

Real
TriEdgeRemesher::triangleQuality(const ShadowMesh & shadow, const ShadowTriangle & t)
{
  return shapeQuality(shadow.vertices[t.vertices[0]].point,
                      shadow.vertices[t.vertices[1]].point,
                      shadow.vertices[t.vertices[2]].point);
}

std::size_t
TriEdgeRemesher::addShadowTriangle(ShadowMesh & shadow,
                                   const std::size_t v0,
                                   const std::size_t v1,
                                   const std::size_t v2,
                                   const subdomain_id_type subdomain,
                                   const std::vector<dof_id_type> & ancestors)
{
  ShadowTriangle triangle;
  triangle.vertices = {v0, v1, v2};
  triangle.subdomain = subdomain;
  triangle.ancestors = ancestors;

  const std::size_t t = shadow.triangles.size();
  shadow.triangles.push_back(triangle);
  for (const auto n : make_range(3u))
  {
    const EdgeKey key = edgeKey(triangle.vertices[n], triangle.vertices[(n + 1) % 3]);
    shadow.edge_triangles[key].push_back(t);
    shadow.vertex_triangles[triangle.vertices[n]].insert(t);
  }
  return t;
}

void
TriEdgeRemesher::mergeAncestors(std::vector<dof_id_type> & ancestors,
                                const std::vector<dof_id_type> & more)
{
  ancestors.insert(ancestors.end(), more.begin(), more.end());
  std::sort(ancestors.begin(), ancestors.end());
  ancestors.erase(std::unique(ancestors.begin(), ancestors.end()), ancestors.end());
}

void
TriEdgeRemesher::removeShadowTriangle(ShadowMesh & shadow, const std::size_t t)
{
  ShadowTriangle & triangle = shadow.triangles[t];
  mooseAssert(triangle.alive, "Triangle " << t << " is removed twice.");
  triangle.alive = false;

  if (triangle.original_id != DofObject::invalid_id)
    shadow.replaced_element_ids.insert(triangle.original_id);

  for (const auto n : make_range(3u))
  {
    const EdgeKey key = edgeKey(triangle.vertices[n], triangle.vertices[(n + 1) % 3]);
    std::vector<std::size_t> & tris = libmesh_map_find(shadow.edge_triangles, key);
    tris.erase(std::remove(tris.begin(), tris.end(), t), tris.end());
    if (tris.empty())
      shadow.edge_triangles.erase(key);
    shadow.vertex_triangles[triangle.vertices[n]].erase(t);
  }
}

unsigned int
TriEdgeRemesher::splitPass(ShadowMesh & shadow) const
{
  // The candidates are snapshot up front, in the increasing key order the map holds them in, so
  // that every rank splits the same edges in the same order. The keys a split adds are left to the
  // next round.
  std::vector<EdgeKey> candidates;
  for (const auto & [key, tris] : shadow.edge_triangles)
  {
    libmesh_ignore(tris);
    const std::optional<Real> length = metricLength(shadow, key.first, key.second);
    if (length && *length > split_bound)
      candidates.push_back(key);
  }

  unsigned int n_splits = 0;
  for (const EdgeKey & key : candidates)
  {
    // An earlier split of a neighboring edge replaced a triangle on this one, but the edge itself
    // is still there unless it was itself split, which a snapshot entry never is twice
    const auto edge_it = shadow.edge_triangles.find(key);
    if (edge_it == shadow.edge_triangles.end())
      continue;

    const auto [a, b] = key;

    // The midpoint vertex. On a constrained edge it lies on the boundary or the seam the edge is
    // part of, so it is locked the way the endpoints are.
    ShadowVertex midpoint;
    midpoint.point = (shadow.vertices[a].point + shadow.vertices[b].point) / 2.0;
    const std::optional<Real> & ta = shadow.vertices[a].target;
    const std::optional<Real> & tb = shadow.vertices[b].target;
    midpoint.target = ta && tb ? std::make_optional((*ta + *tb) / 2.0) : (ta ? ta : tb);
    midpoint.locked = edgeConstrained(shadow, key);
    // The midpoint lies on the edge, so the elements that cover the triangles on it cover it too
    for (const auto t : edge_it->second)
      mergeAncestors(midpoint.ancestors, shadow.triangles[t].ancestors);

    const std::size_t m = shadow.vertices.size();
    shadow.vertices.push_back(midpoint);
    shadow.vertex_triangles.emplace_back();

    // Both halves inherit the boundary ids of the split edge
    const auto ids_it = shadow.edge_boundary_ids.find(key);
    if (ids_it != shadow.edge_boundary_ids.end())
    {
      const std::vector<boundary_id_type> ids = ids_it->second;
      shadow.edge_boundary_ids.erase(ids_it);
      shadow.edge_boundary_ids[edgeKey(a, m)] = ids;
      shadow.edge_boundary_ids[edgeKey(m, b)] = ids;
    }

    // Bisect the one or two triangles on the edge. The children traverse the split side the way
    // the parent did, with the midpoint in between, which preserves the winding.
    const std::vector<std::size_t> tris = edge_it->second;
    for (const auto t : tris)
    {
      const ShadowTriangle parent = shadow.triangles[t];
      removeShadowTriangle(shadow, t);

      for (const auto n : make_range(3u))
      {
        const std::size_t vi = parent.vertices[n];
        const std::size_t vj = parent.vertices[(n + 1) % 3];
        if (edgeKey(vi, vj) != key)
          continue;

        const std::size_t vk = parent.vertices[(n + 2) % 3];
        addShadowTriangle(shadow, vi, m, vk, parent.subdomain, parent.ancestors);
        addShadowTriangle(shadow, m, vj, vk, parent.subdomain, parent.ancestors);
        break;
      }
    }

    ++n_splits;
  }

  return n_splits;
}

unsigned int
TriEdgeRemesher::collapsePass(ShadowMesh & shadow) const
{
  std::vector<EdgeKey> candidates;
  for (const auto & [key, tris] : shadow.edge_triangles)
  {
    libmesh_ignore(tris);
    const std::optional<Real> length = metricLength(shadow, key.first, key.second);
    if (length && *length < collapse_bound)
      candidates.push_back(key);
  }

  unsigned int n_collapses = 0;
  for (const EdgeKey & key : candidates)
  {
    // An earlier collapse can have removed this edge with the vertex it took
    const auto edge_it = shadow.edge_triangles.find(key);
    if (edge_it == shadow.edge_triangles.end())
      continue;

    // Only an interior edge is collapsed: a boundary edge's endpoints are both locked anyway
    if (edge_it->second.size() != 2)
      continue;

    // The opposite vertices of the two triangles on the edge, which are the only vertices the
    // link of a collapsible edge may hold
    std::set<std::size_t> opposite;
    for (const auto t : edge_it->second)
      for (const auto v : shadow.triangles[t].vertices)
        if (v != key.first && v != key.second)
          opposite.insert(v);

    // The higher index is tried first, which prefers removing the midpoints splits created and
    // keeping the original nodes
    for (const auto [removed, kept] :
         {std::make_pair(key.second, key.first), std::make_pair(key.first, key.second)})
    {
      if (shadow.vertices[removed].locked)
        continue;

      // The link condition: a vertex adjacent to both endpoints that is not an opposite vertex
      // would end up in two triangles with the same three vertices, which pinches the mesh
      std::set<std::size_t> removed_neighbors;
      for (const auto t : shadow.vertex_triangles[removed])
        for (const auto v : shadow.triangles[t].vertices)
          if (v != removed)
            removed_neighbors.insert(v);

      bool pinched = false;
      for (const auto v : removed_neighbors)
        if (v != kept && !opposite.count(v) && shadow.edge_triangles.count(edgeKey(v, kept)))
          pinched = true;
      if (pinched)
        continue;

      // Simulate the fan the collapse leaves: every triangle around the removed vertex that does
      // not die retargets the removed vertex at the kept one. The fan is refused when a triangle
      // inverts, degrades below the floor a healthy fan is held to, or acquires an edge the split
      // pass would immediately take back.
      Real quality_floor = collapse_quality_floor;
      for (const auto t : shadow.vertex_triangles[removed])
        quality_floor = std::min(quality_floor, triangleQuality(shadow, shadow.triangles[t]));

      bool valid = true;
      for (const auto t : shadow.vertex_triangles[removed])
      {
        const ShadowTriangle & triangle = shadow.triangles[t];
        const bool dies = std::find(triangle.vertices.begin(), triangle.vertices.end(), kept) !=
                          triangle.vertices.end();
        if (dies)
          continue;

        std::array<std::size_t, 3> vertices = triangle.vertices;
        for (auto & v : vertices)
          if (v == removed)
            v = kept;

        if (shapeQuality(shadow.vertices[vertices[0]].point,
                         shadow.vertices[vertices[1]].point,
                         shadow.vertices[vertices[2]].point) < quality_floor)
        {
          valid = false;
          break;
        }

        for (const auto v : vertices)
        {
          if (v == kept)
            continue;
          const std::optional<Real> length = metricLength(shadow, kept, v);
          if (length && *length > split_bound)
          {
            valid = false;
            break;
          }
        }
        if (!valid)
          break;
      }
      if (!valid)
        continue;

      // Perform the collapse. The star is copied because removing triangles mutates it.
      const std::set<std::size_t> star = shadow.vertex_triangles[removed];

      // The retargeted fan covers the region the star covered, so any element that covered a
      // triangle of the star can cover a retargeted triangle
      std::vector<dof_id_type> star_ancestors;
      for (const auto t : star)
        mergeAncestors(star_ancestors, shadow.triangles[t].ancestors);

      for (const auto t : star)
      {
        const ShadowTriangle triangle = shadow.triangles[t];
        removeShadowTriangle(shadow, t);

        const bool dies = std::find(triangle.vertices.begin(), triangle.vertices.end(), kept) !=
                          triangle.vertices.end();
        if (dies)
          continue;

        std::array<std::size_t, 3> vertices = triangle.vertices;
        for (auto & v : vertices)
          if (v == removed)
            v = kept;
        addShadowTriangle(
            shadow, vertices[0], vertices[1], vertices[2], triangle.subdomain, star_ancestors);
      }

      mooseAssert(shadow.vertex_triangles[removed].empty(),
                  "Vertex " << removed
                            << " still has triangles after its whole star was replaced.");
      shadow.vertices[removed].alive = false;

      ++n_collapses;
      break;
    }
  }

  return n_collapses;
}

unsigned int
TriEdgeRemesher::swapPass(ShadowMesh & shadow) const
{
  std::vector<EdgeKey> candidates;
  for (const auto & [key, tris] : shadow.edge_triangles)
  {
    libmesh_ignore(tris);
    candidates.push_back(key);
  }

  unsigned int n_swaps = 0;
  for (const EdgeKey & key : candidates)
  {
    const auto edge_it = shadow.edge_triangles.find(key);
    if (edge_it == shadow.edge_triangles.end())
      continue;

    if (edge_it->second.size() != 2 || edgeConstrained(shadow, key))
      continue;

    const auto [a, b] = key;

    // The triangle that traverses the edge a then b, and the one that traverses it b then a, with
    // their opposite vertices. Naming them by traversal is what makes the swapped pair wind the
    // way the old pair did.
    std::size_t t_ab = 0;
    std::size_t t_ba = 0;
    std::size_t c = 0;
    std::size_t d = 0;
    for (const auto t : edge_it->second)
    {
      const ShadowTriangle & triangle = shadow.triangles[t];
      for (const auto n : make_range(3u))
      {
        const std::size_t vi = triangle.vertices[n];
        const std::size_t vj = triangle.vertices[(n + 1) % 3];
        if (vi == a && vj == b)
        {
          t_ab = t;
          c = triangle.vertices[(n + 2) % 3];
        }
        else if (vi == b && vj == a)
        {
          t_ba = t;
          d = triangle.vertices[(n + 2) % 3];
        }
      }
    }

    // A swapped edge that already exists would put two triangles on each of its sides
    if (shadow.edge_triangles.count(edgeKey(c, d)))
      continue;

    const Point & pa = shadow.vertices[a].point;
    const Point & pb = shadow.vertices[b].point;
    const Point & pc = shadow.vertices[c].point;
    const Point & pd = shadow.vertices[d].point;

    // Both new triangles counter-clockwise is the quadrilateral being strictly convex, and the
    // swap is only taken when it strictly improves the worse of the two shapes
    const Real old_quality = std::min(triangleQuality(shadow, shadow.triangles[t_ab]),
                                      triangleQuality(shadow, shadow.triangles[t_ba]));
    const Real new_quality = std::min(shapeQuality(pa, pd, pc), shapeQuality(pd, pb, pc));
    if (new_quality <= old_quality + swap_improvement)
      continue;

    mooseAssert(shadow.triangles[t_ab].subdomain == shadow.triangles[t_ba].subdomain,
                "Edge (" << a << ", " << b
                         << ") is unconstrained but its triangles lie in different subdomains.");
    const subdomain_id_type subdomain = shadow.triangles[t_ab].subdomain;

    // Both new triangles lie in the quadrilateral the two old ones covered
    std::vector<dof_id_type> ancestors = shadow.triangles[t_ab].ancestors;
    mergeAncestors(ancestors, shadow.triangles[t_ba].ancestors);

    removeShadowTriangle(shadow, t_ab);
    removeShadowTriangle(shadow, t_ba);
    addShadowTriangle(shadow, a, d, c, subdomain, ancestors);
    addShadowTriangle(shadow, d, b, c, subdomain, ancestors);

    ++n_swaps;
  }

  return n_swaps;
}

void
TriEdgeRemesher::spliceShadow(const ShadowMesh & shadow, RemeshRecord & record) const
{
  MeshBase & mesh = _mesh.getMesh();

  dof_id_type n_new_nodes = 0;
  dof_id_type n_new_elements = 0;
  for (const ShadowVertex & vertex : shadow.vertices)
    if (vertex.alive && !vertex.node)
      ++n_new_nodes;
  for (const ShadowTriangle & triangle : shadow.triangles)
    if (triangle.alive && triangle.original_id == DofObject::invalid_id)
      ++n_new_elements;

  dof_id_type next_node_id = 0;
  dof_id_type next_elem_id = 0;
  reserveNewEntityIds(n_new_nodes, n_new_elements, next_node_id, next_elem_id);

  // The final node of every alive vertex: the node it started as, or the one created for it here,
  // in vertex index order so that the ids depend on the mesh alone
  std::vector<Node *> final_nodes(shadow.vertices.size(), nullptr);
  for (const auto v : index_range(shadow.vertices))
  {
    const ShadowVertex & vertex = shadow.vertices[v];
    if (!vertex.alive)
      continue;

    if (vertex.node)
    {
      final_nodes[v] = vertex.node;
      continue;
    }

    const RemeshSourcePoint source = locateSourcePoint(vertex.ancestors, vertex.point);
    final_nodes[v] = addNode(vertex.point, source, next_node_id, record);
  }

  // The boundary ids of every edge that carries any, keyed by the sorted node id pair the way
  // addTriangle looks sides up
  SideBoundaryIds side_boundary_ids;
  for (const auto & [key, ids] : shadow.edge_boundary_ids)
  {
    mooseAssert(final_nodes[key.first] && final_nodes[key.second],
                "Edge (" << key.first << ", " << key.second
                         << ") carries boundary ids but one of its vertices is gone.");
    side_boundary_ids[sortedNodePair(final_nodes[key.first]->id(), final_nodes[key.second]->id())] =
        ids;
  }

  for (const ShadowTriangle & triangle : shadow.triangles)
  {
    if (!triangle.alive || triangle.original_id != DofObject::invalid_id)
      continue;

    const std::array<Node *, 3> nodes{final_nodes[triangle.vertices[0]],
                                      final_nodes[triangle.vertices[1]],
                                      final_nodes[triangle.vertices[2]]};

    Point centroid;
    for (const Node * node : nodes)
      centroid += *node;
    centroid /= 3.0;

    const RemeshSourcePoint source = locateSourcePoint(triangle.ancestors, centroid);
    mooseAssert(source.old_elem->subdomain_id() == triangle.subdomain,
                "The centroid of a new triangle of subdomain "
                    << triangle.subdomain << " was located in old element " << source.old_elem->id()
                    << " of subdomain " << source.old_elem->subdomain_id()
                    << ", but every operation stays inside one subdomain.");

    addMirroredTriangle(nodes, source, side_boundary_ids, next_elem_id, record);
  }

  // The replaced entities stay in the mesh. The engine still has to read the old solution through
  // them, erase their stateful material properties and only then free them.
  for (const auto elem_id : shadow.replaced_element_ids)
    record.replaced_elements.push_back(mesh.elem_ptr(elem_id));

  for (const ShadowVertex & vertex : shadow.vertices)
    if (!vertex.alive && vertex.node)
      record.replaced_nodes.push_back(vertex.node);
}
