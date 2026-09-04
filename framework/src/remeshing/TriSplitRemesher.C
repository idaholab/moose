//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriSplitRemesher.h"

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
#include "libmesh/remote_elem.h"

#include <algorithm>

using namespace libMesh;

namespace
{
/// Reference (master element) coordinates of the three vertices of a TRI3
const std::array<Point, 3> tri3_vertex_xi{Point(0, 0), Point(1, 0), Point(0, 1)};

/**
 * Reference (master element) coordinate of the midpoint of each side of a TRI3. Side s of a TRI3
 * runs from local node s to local node (s + 1) % 3, which is the convention every local index in
 * this file follows.
 */
const std::array<Point, 3> tri3_midpoint_xi{Point(0.5, 0), Point(0.5, 0.5), Point(0, 0.5)};

/**
 * The element across side \p side of \p elem, null when the side has no neighbor and null as well
 * when the neighbor is not in this rank's copy of the mesh, which no rank local marking can reach.
 */
const Elem *
localNeighbor(const Elem & elem, const unsigned int side)
{
  const Elem * neighbor = elem.neighbor_ptr(side);
  return neighbor == remote_elem ? nullptr : neighbor;
}
}

registerMooseObject("MooseApp", TriSplitRemesher);

InputParameters
TriSplitRemesher::validParams()
{
  InputParameters params = Remesher::validParams();
  params.addClassDescription(
      "Splits the triangles that are larger than a target element size field into four, closing "
      "the split with green bisections so that the mesh keeps no hanging node.");

  params.addRequiredParam<VariableName>(
      "sizing_variable",
      "Target element size field, as a CONSTANT MONOMIAL variable that carries one target size per "
      "element. A triangle whose longest edge is longer than the target on it is split.");
  params.addRangeCheckedParam<Real>(
      "min_element_size",
      "min_element_size > 0",
      "Floor the target element size is held at: where the sizing variable asks for less, the "
      "floor takes over, and the variable may then fall to zero or below there. When this is not "
      "set the sizing variable itself must be positive on every element.");
  params.addRangeCheckedParam<unsigned int>(
      "max_splits_per_event",
      "max_splits_per_event > 0",
      "Largest number of oversized elements one remesh event splits, taking the ones with the "
      "lowest element ids first. The elements the closure has to split with them are not counted "
      "against it, because dropping those would leave hanging nodes. All of the oversized elements "
      "are split when this is not set.");

  return params;
}

TriSplitRemesher::TriSplitRemesher(const InputParameters & parameters)
  : Remesher(parameters),
    _sizing_variable(_fe_problem.getVariable(/*tid=*/0,
                                             getParam<VariableName>("sizing_variable"),
                                             Moose::VarKindType::VAR_ANY,
                                             Moose::VarFieldType::VAR_FIELD_STANDARD)),
    _min_element_size(isParamValid("min_element_size")
                          ? std::make_optional(getParam<Real>("min_element_size"))
                          : std::nullopt),
    _max_splits_per_event(isParamValid("max_splits_per_event")
                              ? std::make_optional(getParam<unsigned int>("max_splits_per_event"))
                              : std::nullopt)
{
  if (_mesh.dimension() != 2)
    mooseError("This remesher splits triangles in the XY plane, but the mesh is ",
               _mesh.dimension(),
               "D.");

  checkElementalSizingVariable("sizing_variable", _sizing_variable);

  // INVALID_ELEM is the last element type, so the reduction reports a type that some rank found and
  // leaves the value alone when no rank found one
  unsigned int other_type = INVALID_ELEM;
  for (const Elem * elem : _mesh.getMesh().active_element_ptr_range())
    if (elem->type() != TRI3)
    {
      other_type = elem->type();
      break;
    }
  _communicator.min(other_type);

  if (other_type != INVALID_ELEM)
    mooseError("This remesher splits TRI3 elements, but the mesh also contains ",
               Utility::enum_to_string(static_cast<ElemType>(other_type)),
               " elements. Build the mesh out of triangles to remesh it this way.");
}

Remesher::RemeshRecord
TriSplitRemesher::remesh()
{
  RemeshRecord record;

  MeshBase & mesh = _mesh.getMesh();

  std::vector<dof_id_type> seeds = selectOversizedElements();
  if (_max_splits_per_event && seeds.size() > *_max_splits_per_event)
    seeds.resize(*_max_splits_per_event);

  // Deferring a red can make a red that was only promoted to close it unnecessary, so the whole
  // pattern is rebuilt from the reduced seed set. Every rebuild defers at least one more element of
  // a set that no element ever leaves and that holds local elements only, so this many rebuilds
  // always reach a pattern that closes. Nothing is ever deferred on a replicated mesh, where every
  // element is one this rank may replace, which is why a single pass suffices there.
  const dof_id_type max_rebuilds = (_mesh.isDistributedMesh() ? mesh.n_active_local_elem() : 0) + 1;

  std::set<dof_id_type> deferred;
  RefinementPattern pattern;
  bool closed = false;
  for (const auto rebuild : make_range(max_rebuilds))
  {
    libmesh_ignore(rebuild);
    closed = buildRefinementPattern(seeds, deferred, pattern);
    if (closed)
      break;
  }
  mooseAssert(closed,
              "The red-green closure deferred an element on every one of its "
                  << max_rebuilds
                  << " rebuilds, which it cannot do: it only ever removes elements from a set "
                     "bounded by the local elements, and the empty set closes.");

  // Deferring a red leaves the mesh alone where the criterion asked for a finer one, which is
  // indistinguishable from the criterion never having fired, so the deferrals are reported
  if (!deferred.empty())
    _console << "Remeshing: deferred " << deferred.size()
             << " oversized elements whose split would have reached onto another rank" << std::endl;

  // reserveNewEntityIds() is collective on a distributed mesh, so every rank reaches it, including
  // one whose own pattern is empty
  dof_id_type next_node_id = 0;
  dof_id_type next_elem_id = 0;
  // One midpoint per split edge, four children per red and two per green
  reserveNewEntityIds(cast_int<dof_id_type>(pattern.split_edges.size()),
                      cast_int<dof_id_type>(4 * pattern.red.size() + 2 * pattern.green.size()),
                      next_node_id,
                      next_elem_id);

  std::map<EdgeKey, Node *> midpoint_nodes;
  createMidpoints(pattern, next_node_id, midpoint_nodes, record);

  // The reds first and then the greens, each in increasing id order, so that the ids the event
  // hands out depend on the mesh alone
  for (const auto elem_id : pattern.red)
    spliceElement(*mesh.elem_ptr(elem_id), midpoint_nodes, next_elem_id, record);
  for (const auto elem_id : pattern.green)
    spliceElement(*mesh.elem_ptr(elem_id), midpoint_nodes, next_elem_id, record);

  record.changed = !record.new_elements.empty();
  return record;
}

TriSplitRemesher::EdgeKey
TriSplitRemesher::edgeKey(const Elem & elem, const unsigned int side)
{
  const auto side_nodes = elem.nodes_on_side(side);
  return sortedNodePair(elem.node_id(side_nodes[0]), elem.node_id(side_nodes[1]));
}

unsigned int
TriSplitRemesher::countSplitSides(const Elem & elem, const std::set<EdgeKey> & split_edges)
{
  unsigned int n_split = 0;
  for (const auto side : elem.side_index_range())
    if (split_edges.count(edgeKey(elem, side)))
      ++n_split;

  return n_split;
}

std::vector<dof_id_type>
TriSplitRemesher::selectOversizedElements() const
{
  const MeshBase & mesh = _mesh.getMesh();
  const bool distributed = _mesh.isDistributedMesh();

  std::vector<dof_id_type> oversized;
  for (const Elem * elem : mesh.active_element_ptr_range())
  {
    mooseAssert(elem->type() == TRI3,
                "Element " << elem->id()
                           << " is not a TRI3, which the constructor rejected the mesh over unless "
                              "the surgery itself created it, and the surgery only creates TRI3s.");

    // A rank measures only the elements it owns, on a replicated mesh as much as on a distributed
    // one: current_local_solution is readable at the owned degrees of freedom plus the send list,
    // and the degrees of freedom partition by processor id even where the elements do not
    if (elem->processor_id() != mesh.processor_id())
      continue;

    const std::optional<Real> target = readTargetSize(_sizing_variable, *elem, _min_element_size);
    if (!target)
      continue;

    if (elem->hmax() > *target)
      oversized.push_back(elem->id());
  }

  // Every rank of a replicated mesh performs the whole surgery, so it needs what the other ranks
  // measured. It gathers nothing on a run of one rank, which owns every element, and it is what
  // keeps the ranks of a larger replicated run selecting the same elements.
  if (!distributed)
    _communicator.allgather(oversized);

  // Ordered by id rather than by the order the mesh iterator produced, so that every rank of a
  // replicated mesh performs the same surgery in the same order and the copies stay identical
  std::sort(oversized.begin(), oversized.end());
  return oversized;
}

bool
TriSplitRemesher::mayRefine(const Elem & elem) const
{
  // Every element of a replicated mesh is one this rank may replace, and every rank replaces the
  // same ones, so the split of any element closes without leaving the rank
  if (!_mesh.isDistributedMesh())
    return true;

  const MeshBase & mesh = _mesh.getMesh();
  if (elem.processor_id() != mesh.processor_id())
    return false;

  for (const auto side : elem.side_index_range())
  {
    const Elem * neighbor = elem.neighbor_ptr(side);
    if (!neighbor)
      continue;

    if (neighbor == remote_elem || neighbor->processor_id() != mesh.processor_id())
      return false;
  }

  return true;
}

bool
TriSplitRemesher::buildRefinementPattern(const std::vector<dof_id_type> & seeds,
                                         std::set<dof_id_type> & deferred,
                                         RefinementPattern & pattern) const
{
  const MeshBase & mesh = _mesh.getMesh();

  pattern.red.clear();
  pattern.green.clear();
  pattern.split_edges.clear();

  // The reds whose sides have not been offered to their neighbors yet
  std::vector<dof_id_type> pending;
  for (const auto seed : seeds)
  {
    if (deferred.count(seed))
      continue;

    if (!mayRefine(*mesh.elem_ptr(seed)))
    {
      deferred.insert(seed);
      continue;
    }

    pattern.red.insert(seed);
    pending.push_back(seed);
  }

  while (!pending.empty())
  {
    for (const auto elem_id : pending)
    {
      const Elem & elem = *mesh.elem_ptr(elem_id);
      for (const auto side : elem.side_index_range())
        pattern.split_edges.insert(edgeKey(elem, side));
    }

    // A candidate is a neighbor of a red that is not red itself. They are collected in an ordered
    // set so that the promotions of a round are decided in increasing id order.
    std::set<dof_id_type> candidates;
    for (const auto elem_id : pending)
    {
      const Elem & elem = *mesh.elem_ptr(elem_id);
      for (const auto side : elem.side_index_range())
      {
        const Elem * neighbor = localNeighbor(elem, side);
        if (neighbor && !pattern.red.count(neighbor->id()))
          candidates.insert(neighbor->id());
      }
    }

    pending.clear();
    for (const auto candidate_id : candidates)
    {
      const Elem & candidate = *mesh.elem_ptr(candidate_id);
      if (countSplitSides(candidate, pattern.split_edges) < 2)
        continue;

      // One bisection cannot close two hanging nodes, so a candidate that acquired two midpoints is
      // promoted to a red of its own, which puts a midpoint on its third side as well
      if (!deferred.count(candidate_id) && mayRefine(candidate))
      {
        pattern.red.insert(candidate_id);
        pending.push_back(candidate_id);
        continue;
      }

      // The promotion is not this rank's to make, so the reds that demanded it are deferred and the
      // caller rebuilds the pattern without them
      for (const auto side : candidate.side_index_range())
      {
        const Elem * neighbor = localNeighbor(candidate, side);
        if (neighbor && pattern.red.count(neighbor->id()) &&
            pattern.split_edges.count(edgeKey(candidate, side)))
          deferred.insert(neighbor->id());
      }

      return false;
    }
  }

  // A neighbor of a red that is not red itself carries exactly one midpoint by now, because a
  // second one would have promoted it, and one hanging node is what a bisection closes
  for (const auto elem_id : pattern.red)
  {
    const Elem & elem = *mesh.elem_ptr(elem_id);
    for (const auto side : elem.side_index_range())
    {
      const Elem * neighbor = localNeighbor(elem, side);
      if (!neighbor || pattern.red.count(neighbor->id()))
        continue;

      mooseAssert(countSplitSides(*neighbor, pattern.split_edges) == 1,
                  "Element " << neighbor->id() << " neighbors a red across a split side but has "
                             << countSplitSides(*neighbor, pattern.split_edges)
                             << " split sides, so the closure would leave a hanging node.");
      pattern.green.insert(neighbor->id());
    }
  }

  return true;
}

void
TriSplitRemesher::createMidpoints(const RefinementPattern & pattern,
                                  dof_id_type & next_node_id,
                                  std::map<EdgeKey, Node *> & midpoint_nodes,
                                  RemeshRecord & record) const
{
  MeshBase & mesh = _mesh.getMesh();

  for (const auto elem_id : pattern.red)
  {
    Elem & parent = *mesh.elem_ptr(elem_id);
    for (const auto side : parent.side_index_range())
    {
      const EdgeKey key = edgeKey(parent, side);
      if (midpoint_nodes.count(key))
        continue;

      const auto side_nodes = parent.nodes_on_side(side);
      const Point point = (parent.point(side_nodes[0]) + parent.point(side_nodes[1])) / 2.0;

      RemeshSourcePoint source;
      source.old_elem = &parent;
      source.xi = tri3_midpoint_xi[side];

      midpoint_nodes[key] = addNode(point, source, next_node_id, record);
    }
  }
}

std::map<TriSplitRemesher::EdgeKey, std::vector<boundary_id_type>>
TriSplitRemesher::sideBoundaryIds(const Elem & elem,
                                  const std::map<EdgeKey, Node *> & midpoint_nodes) const
{
  const BoundaryInfo & boundary_info = _mesh.getMesh().get_boundary_info();

  std::map<EdgeKey, std::vector<boundary_id_type>> side_boundary_ids;
  std::vector<boundary_id_type> side_ids;

  for (const auto side : elem.side_index_range())
  {
    boundary_info.boundary_ids(&elem, side, side_ids);
    if (side_ids.empty())
      continue;

    const auto side_nodes = elem.nodes_on_side(side);
    const dof_id_type first = elem.node_id(side_nodes[0]);
    const dof_id_type second = elem.node_id(side_nodes[1]);

    // A side with no midpoint is not split, so the child that inherits it inherits the whole side
    const auto it = midpoint_nodes.find(sortedNodePair(first, second));
    if (it == midpoint_nodes.end())
    {
      side_boundary_ids[sortedNodePair(first, second)] = side_ids;
      continue;
    }

    const dof_id_type midpoint = it->second->id();
    side_boundary_ids[sortedNodePair(first, midpoint)] = side_ids;
    side_boundary_ids[sortedNodePair(midpoint, second)] = side_ids;
  }

  return side_boundary_ids;
}

std::vector<TriSplitRemesher::ChildTriangle>
TriSplitRemesher::childTriangles(Elem & parent, const std::array<Node *, 3> & midpoints)
{
  std::array<ChildVertex, 3> vertex;
  for (const auto n : index_range(vertex))
    vertex[n] = ChildVertex{parent.node_ptr(n), tri3_vertex_xi[n]};

  std::array<ChildVertex, 3> midpoint;
  for (const auto s : index_range(midpoint))
    midpoint[s] = ChildVertex{midpoints[s], tri3_midpoint_xi[s]};

  const auto n_split = std::count_if(
      midpoints.begin(), midpoints.end(), [](const Node * node) { return node != nullptr; });

  // The four children of a red, which are the three corner triangles and the medial triangle. They
  // wind the way the parent does, because a corner triangle keeps two of the parent's directions
  // and the medial triangle is the parent scaled by minus one half about its centroid.
  if (n_split == 3)
    return {ChildTriangle{vertex[0], midpoint[0], midpoint[2]},
            ChildTriangle{midpoint[0], vertex[1], midpoint[1]},
            ChildTriangle{midpoint[2], midpoint[1], vertex[2]},
            ChildTriangle{midpoint[0], midpoint[1], midpoint[2]}};

  mooseAssert(n_split == 1,
              "Element " << parent.id() << " is replaced with " << n_split
                         << " of its sides split, but only a red with three split sides and a "
                            "green with one are ever replaced.");

  // The two children of a green, which are the halves the segment from the midpoint of the split
  // side to the vertex opposite it cuts the parent into
  const unsigned int split_side = midpoints[0] ? 0 : (midpoints[1] ? 1 : 2);
  const unsigned int opposite = (split_side + 2) % 3;
  return {ChildTriangle{vertex[split_side], midpoint[split_side], vertex[opposite]},
          ChildTriangle{midpoint[split_side], vertex[(split_side + 1) % 3], vertex[opposite]}};
}

void
TriSplitRemesher::spliceElement(Elem & parent,
                                const std::map<EdgeKey, Node *> & midpoint_nodes,
                                dof_id_type & next_elem_id,
                                RemeshRecord & record) const
{
  mooseAssert(!_mesh.isDistributedMesh() || parent.processor_id() == _mesh.getMesh().processor_id(),
              "Element " << parent.id()
                         << " is replaced by a rank that does not own it, so this rank is about to "
                            "delete an element another rank owns.");

  // A green carries a midpoint on its one split side only, and a red on all three of its sides, so
  // the lookup tells the two apart without the marks being consulted again
  std::array<Node *, 3> midpoints{nullptr, nullptr, nullptr};
  for (const auto side : parent.side_index_range())
  {
    const auto it = midpoint_nodes.find(edgeKey(parent, side));
    if (it != midpoint_nodes.end())
      midpoints[side] = it->second;
  }

  const auto side_boundary_ids = sideBoundaryIds(parent, midpoint_nodes);

  for (const auto & child : childTriangles(parent, midpoints))
  {
    const std::array<Node *, 3> nodes{child[0].node, child[1].node, child[2].node};

    RemeshSourcePoint source;
    source.old_elem = &parent;
    // The map of a TRI3 is affine, so the centroid of a child sits at the average of the reference
    // coordinates of its three vertices
    source.xi = (child[0].xi + child[1].xi + child[2].xi) / 3.0;

    addMirroredTriangle(nodes, source, side_boundary_ids, next_elem_id, record);
  }

  // The parent stays in the mesh. The engine still has to read the old solution through it, erase
  // its stateful material properties and only then free it.
  record.replaced_elements.push_back(&parent);
}
