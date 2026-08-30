//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTreeCotreeGauge.h"
#include "MFEMDistributedGraph.h"
#include "MFEMEssentialConstraint.h"
#include "libmesh/int_range.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <vector>

namespace
{

// ---------------------------------------------------------------------
// Minimal union-find (disjoint set) helper used to grow the spanning
// forest. Operates on dense 0-based vertex ids.
// ---------------------------------------------------------------------
class UnionFind
{
public:
  explicit UnionFind(int n) : _parent(n), _rank(n, 0)
  {
    for (const auto i : make_range(n))
      _parent[i] = i;
  }

  int find(int x)
  {
    while (_parent[x] != x)
    {
      _parent[x] = _parent[_parent[x]];
      x = _parent[x];
    }
    return x;
  }

  // Returns true if a and b were in different components, i.e. the edge
  // between them closes no cycle and can join the spanning forest.
  bool join(int a, int b)
  {
    a = find(a);
    b = find(b);
    if (a == b)
      return false;
    if (_rank[a] < _rank[b])
      std::swap(a, b);
    _parent[b] = a;
    if (_rank[a] == _rank[b])
      _rank[a]++;
    return true;
  }

private:
  std::vector<int> _parent, _rank;
};

// A vertex location, used as a partition-independent vertex identity.
using Coord3 = std::array<mfem::real_t, 3>;

// Lowest-order edge dof of parallel mesh edge e, sign-decoded. Called once per
// mesh edge in each pass below, so the caller supplies the dof scratch space
// rather than having it reallocated for every edge.
int
edgeLDof(mfem::ParFiniteElementSpace & pfes, int e, mfem::Array<int> & dofs)
{
  pfes.GetEdgeDofs(e, dofs);
  return (dofs[0] >= 0) ? dofs[0] : -1 - dofs[0];
}

Coord3
vertexCoord(const mfem::ParMesh & pmesh, int v)
{
  Coord3 p{0.0, 0.0, 0.0};
  const mfem::real_t * x = pmesh.GetVertex(v);
  for (const auto d : make_range(pmesh.SpaceDimension()))
    p[d] = x[d];
  return p;
}

// Per-local-edge marker: 1 if the edge belongs to an element whose attribute is
// NOT among gauge_attrs, i.e. an edge of a region the gauge must not reach because
// some other term of the weak form already removes the gradient null space there.
// These edges seed the spanning forest but are never gauged. Edges of a gauged
// region that merely touch such a region are NOT flagged, so the gauge still
// reaches the vertices next to it. Consistent across ranks for shared edges. An
// empty gauge_attrs gauges the whole mesh and flags nothing.
//
// The solve space is required to be order 1, so its own dofs are exactly the mesh
// edges and it can supply both the element/edge adjacency and the group
// communicator; no auxiliary space is needed.
mfem::Array<int>
excludedEdgeMarker(mfem::ParFiniteElementSpace & pfes, const mfem::Array<int> & gauge_attrs)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();
  mfem::Array<int> excluded(pmesh.GetNEdges());
  excluded = 0;
  if (gauge_attrs.Size() == 0)
    return excluded;

  const int max_attr = pmesh.attributes.Size() ? pmesh.attributes.Max() : 0;
  std::vector<char> is_gauge_attr(max_attr + 1, 0);
  for (const auto a : gauge_attrs)
    if (a >= 1 && a <= max_attr)
      is_gauge_attr[a] = 1;

  mfem::Array<int> marker(pfes.GetVSize());
  marker = 0;
  mfem::Array<int> edofs;
  for (const auto el : make_range(pmesh.GetNE()))
    if (!is_gauge_attr[pmesh.GetAttribute(el)])
    {
      pfes.GetElementDofs(el, edofs);
      for (const auto d : edofs)
        marker[d < 0 ? -1 - d : d] = 1;
    }
  pfes.Synchronize(marker); // boolean-OR over shared edges

  for (const auto e : make_range(pmesh.GetNEdges()))
    excluded[e] = marker[edgeLDof(pfes, e, edofs)];
  return excluded;
}

// One of this rank's owned mesh edges, keyed on its endpoint coordinates.
struct OwnedEdge
{
  Coord3 p0, p1;
  /// Index into the local ParMesh edge numbering, used to recover the ND dof.
  int mesh_edge;
  /// Seeds the forest (essential boundary or excluded region) and is never gauged.
  bool seed;
};

// A candidate edge once its endpoints carry canonical global vertex ids.
struct CandidateEdge
{
  /// Canonical global endpoint ids, wu < wv. These order the edges.
  std::int64_t wu, wv;
  /// The same endpoints in this rank's dense local numbering, so the local cycle
  /// filter needs no lookup.
  int lu, lv;
  int mesh_edge;
};

// Round-trip record for looking up the canonical id of a coordinate.
struct CoordQuery
{
  Coord3 coord;
  std::int64_t id;
  int origin_rank, origin_index;
};

// Canonical numbering of the vertex coordinates handed to canonicalVertexIds.
struct VertexNumbering
{
  /// Global canonical id of each input coordinate, in input order.
  std::vector<std::int64_t> global_id;
  /// Index of each input coordinate in this rank's sorted distinct list, i.e. a
  /// dense local numbering in [0, num_local).
  std::vector<int> local_index;
  int num_local = 0;
  std::int64_t num_global = 0;
};

// ---------------------------------------------------------------------
// Assign every distinct endpoint coordinate in the mesh a dense global id
// in the canonical (lexicographically sorted) order of the whole coordinate
// set. That order is a property of the set alone, so the ids - and hence the
// spanning forest built from them - are the same however the mesh is
// partitioned, without any rank ever holding the whole set.
//
// Vertex coordinates are copied verbatim from the serial mesh when it is
// partitioned, so they are bit-identical on every rank and give a
// partition-independent vertex identity.
// ---------------------------------------------------------------------
VertexNumbering
canonicalVertexIds(const std::vector<Coord3> & coords, MPI_Comm comm)
{
  int nprocs, rank;
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);

  VertexNumbering numbering;

  // Sorting (coordinate, input position) once yields both the distinct list and
  // the mapping from every input coordinate onto it, so no coordinate ever has to
  // be looked up by binary search afterwards.
  std::vector<std::pair<Coord3, int>> sorted(coords.size());
  for (const auto i : index_range(coords))
    sorted[i] = {coords[i], static_cast<int>(i)};
  std::sort(sorted.begin(), sorted.end());

  std::vector<Coord3> distinct;
  numbering.local_index.resize(coords.size());
  for (const auto & [coord, position] : sorted)
  {
    if (distinct.empty() || distinct.back() != coord)
      distinct.push_back(coord);
    numbering.local_index[position] = static_cast<int>(distinct.size()) - 1;
  }
  sorted.clear();
  sorted.shrink_to_fit();
  numbering.num_local = static_cast<int>(distinct.size());

  // Splitters that cut the globally sorted coordinate order into one bucket per
  // rank. Every rank derives them from the same gathered sample, so they agree.
  // Their exact placement only affects load balance: the ids below come from the
  // global sorted position, which no choice of splitters can change.
  constexpr std::size_t oversample = 32;
  const std::size_t num_samples = std::min(oversample, distinct.size());
  std::vector<Coord3> sample;
  for (const auto i : make_range(num_samples))
    sample.push_back(distinct[i * distinct.size() / num_samples]);

  int sample_count = static_cast<int>(sample.size());
  std::vector<int> sample_counts(nprocs), sample_displs(nprocs, 0);
  MPI_Allgather(&sample_count, 1, MPI_INT, sample_counts.data(), 1, MPI_INT, comm);
  std::partial_sum(sample_counts.begin(), sample_counts.end() - 1, sample_displs.begin() + 1);

  MPI_Datatype coord_type;
  MPI_Type_contiguous(sizeof(Coord3), MPI_BYTE, &coord_type);
  MPI_Type_commit(&coord_type);
  std::vector<Coord3> all_samples(static_cast<std::size_t>(sample_displs.back()) +
                                  sample_counts.back());
  MPI_Allgatherv(sample.data(),
                 sample_count,
                 coord_type,
                 all_samples.data(),
                 sample_counts.data(),
                 sample_displs.data(),
                 coord_type,
                 comm);
  MPI_Type_free(&coord_type);
  std::sort(all_samples.begin(), all_samples.end());

  std::vector<Coord3> splitters;
  for (const auto p : make_range(nprocs - 1))
    if (!all_samples.empty())
      splitters.push_back(all_samples[(p + 1) * all_samples.size() / nprocs]);
  auto bucketOf = [&](const Coord3 & c)
  {
    return static_cast<int>(std::upper_bound(splitters.begin(), splitters.end(), c) -
                            splitters.begin());
  };

  // Send each distinct local coordinate to the rank owning its bucket.
  std::vector<CoordQuery> outgoing(distinct.size());
  for (const auto i : index_range(distinct))
    outgoing[i] = {distinct[i], 0, rank, static_cast<int>(i)};

  std::vector<int> counts;
  const auto grouped = Moose::MFEM::groupByDestination(
      outgoing, nprocs, [&](const CoordQuery & q) { return bucketOf(q.coord); }, counts);
  auto received = Moose::MFEM::allToAll(grouped, counts, comm);

  // The distinct coordinates of this bucket, in order, are a contiguous slice of
  // the global canonical order; an exclusive scan of the slice sizes gives its
  // starting id.
  std::vector<Coord3> owned;
  owned.reserve(received.size());
  for (const auto & q : received)
    owned.push_back(q.coord);
  std::sort(owned.begin(), owned.end());
  owned.erase(std::unique(owned.begin(), owned.end()), owned.end());

  std::int64_t owned_count = static_cast<std::int64_t>(owned.size()), offset = 0;
  MPI_Exscan(&owned_count, &offset, 1, MPI_INT64_T, MPI_SUM, comm);
  if (rank == 0)
    offset = 0;
  MPI_Allreduce(&owned_count, &numbering.num_global, 1, MPI_INT64_T, MPI_SUM, comm);

  for (auto & q : received)
    q.id = offset + (std::lower_bound(owned.begin(), owned.end(), q.coord) - owned.begin());

  std::vector<int> answer_counts;
  const auto answers = Moose::MFEM::groupByDestination(
      received, nprocs, [](const CoordQuery & q) { return q.origin_rank; }, answer_counts);

  std::vector<std::int64_t> distinct_ids(distinct.size());
  for (const auto & q : Moose::MFEM::allToAll(answers, answer_counts, comm))
    distinct_ids[q.origin_index] = q.id;

  // Spread the distinct ids back over the caller's coordinate list.
  numbering.global_id.resize(coords.size());
  for (const auto i : index_range(coords))
    numbering.global_id[i] = distinct_ids[numbering.local_index[i]];
  return numbering;
}

// ---------------------------------------------------------------------
// Discard edges that close a cycle among this rank's own edges. By the cycle
// property such an edge is the heaviest on that cycle, so it cannot belong to
// the minimum spanning forest of the whole graph either, and dropping it here
// keeps it out of every subsequent communication round. Contracting the seed
// components later can only shorten those cycles, never break them, so the
// filter stays valid for the contracted graph too.
//
// Returns the surviving edges; @p num_local is the number of distinct vertices in
// this rank's dense local numbering.
// ---------------------------------------------------------------------
std::vector<CandidateEdge>
filterLocalCycles(std::vector<CandidateEdge> candidates, int num_local)
{
  // Kruskal in the canonical edge order, so the survivors are exactly those a
  // serial pass would still have to consider.
  std::sort(candidates.begin(),
            candidates.end(),
            [](const CandidateEdge & a, const CandidateEdge & b)
            { return (a.wu != b.wu) ? a.wu < b.wu : a.wv < b.wv; });

  UnionFind uf(num_local);
  std::vector<CandidateEdge> kept;
  for (const auto & c : candidates)
    if (uf.join(c.lu, c.lv))
      kept.push_back(c);
  return kept;
}

// ---------------------------------------------------------------------
// Build the interior tree-cotree gauge as a list of this rank's ND true
// dofs that must be strongly set to zero, in addition to those already
// fixed by an essential (tangential Dirichlet) boundary condition.
//
// The forest is grown with a distributed Boruvka pass over the mesh
// 1-skeleton: no rank ever holds more than its own share of the edges. Edges
// are ordered by their canonical global endpoint ids, and because that order
// is a total order on distinct weights the minimum spanning forest is unique.
// The gauge - and hence the solution - therefore does not depend on the number
// of MPI ranks or on how the mesh was partitioned.
//
//   pfes           - the order 1 ND space being solved on
//   ess_bdr        - boundary attribute marker for the essential condition
//   essential_tdofs - true dofs already made essential by ess_bdr
//   edge_excluded  - per-local-edge marker of edges the gauge must not touch;
//                    they seed the forest like the essential boundary but are
//                    never gauged
// ---------------------------------------------------------------------
mfem::Array<int>
buildTreeCotreeGaugeTDofs(mfem::ParFiniteElementSpace & pfes,
                          const mfem::Array<int> & ess_bdr,
                          const mfem::Array<int> & essential_tdofs,
                          const mfem::Array<int> & edge_excluded)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();
  MPI_Comm comm = pmesh.GetComm();
  int nprocs, rank;
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);

  // Lowest-order edge dofs fixed by the essential boundary condition.
  mfem::Array<int> ess_vdof_marker;
  pfes.GetEssentialVDofs(ess_bdr, ess_vdof_marker);

  // 1. This rank's owned edges. Each mesh edge is owned by exactly one rank, so
  //    the union over ranks is the mesh 1-skeleton with no duplicates.
  std::vector<OwnedEdge> owned_edges;
  mfem::Array<int> ev, edge_dofs;
  for (const auto e : make_range(pmesh.GetNEdges()))
  {
    const int ldof = edgeLDof(pfes, e, edge_dofs);
    if (pfes.GetLocalTDofNumber(ldof) < 0)
      continue; // owned by another rank; it will contribute this edge

    pmesh.GetEdgeVertices(e, ev);
    owned_edges.push_back({vertexCoord(pmesh, ev[0]),
                           vertexCoord(pmesh, ev[1]),
                           e,
                           ess_vdof_marker[ldof] != 0 || edge_excluded[e] != 0});
  }

  // 2. Canonical global vertex ids for both endpoints of every owned edge.
  std::vector<Coord3> endpoints;
  endpoints.reserve(2 * owned_edges.size());
  for (const auto & oe : owned_edges)
  {
    endpoints.push_back(oe.p0);
    endpoints.push_back(oe.p1);
  }
  const auto numbering = canonicalVertexIds(endpoints, comm);
  const std::int64_t num_vertices = numbering.num_global;
  endpoints.clear();
  endpoints.shrink_to_fit();

  std::vector<CandidateEdge> seed_edges, free_edges;
  for (const auto i : index_range(owned_edges))
  {
    const std::int64_t a = numbering.global_id[2 * i], b = numbering.global_id[2 * i + 1];
    const int la = numbering.local_index[2 * i], lb = numbering.local_index[2 * i + 1];
    // Order both representations of the endpoints the same way.
    const CandidateEdge c = (a < b) ? CandidateEdge{a, b, la, lb, owned_edges[i].mesh_edge}
                                    : CandidateEdge{b, a, lb, la, owned_edges[i].mesh_edge};
    (owned_edges[i].seed ? seed_edges : free_edges).push_back(c);
  }
  owned_edges.clear();
  owned_edges.shrink_to_fit();

  // 3. Drop locally redundant edges before any communication.
  seed_edges = filterLocalCycles(std::move(seed_edges), numbering.num_local);
  free_edges = filterLocalCycles(std::move(free_edges), numbering.num_local);

  // 4. Ground the seeded regions: the connected components of the essential
  //    boundary and of the excluded subdomains, which are fixed without being
  //    gauged.
  std::vector<Moose::MFEM::DistributedEdge> seed_graph(seed_edges.size());
  for (const auto i : index_range(seed_edges))
    seed_graph[i] = {seed_edges[i].wu,
                     seed_edges[i].wv,
                     seed_edges[i].wu,
                     seed_edges[i].wv,
                     rank,
                     static_cast<int>(i)};

  std::vector<std::int64_t> seed_labels;
  Moose::MFEM::distributedSpanningForest(seed_graph, num_vertices, comm, nullptr, &seed_labels);

  // 5. Spanning forest of the remaining edges over the contracted seed
  //    components. An edge whose endpoints already share a component closes a
  //    cycle and is left free (a cotree edge).
  const Moose::MFEM::BlockDistribution dist(num_vertices, nprocs);
  std::vector<std::int64_t> free_endpoints;
  free_endpoints.reserve(2 * free_edges.size());
  for (const auto & c : free_edges)
  {
    free_endpoints.push_back(c.wu);
    free_endpoints.push_back(c.wv);
  }
  const auto free_labels = num_vertices
                               ? Moose::MFEM::fetchLabels(free_endpoints, seed_labels, dist, comm)
                               : std::vector<std::int64_t>();

  std::vector<Moose::MFEM::DistributedEdge> free_graph;
  free_graph.reserve(free_edges.size());
  for (const auto i : index_range(free_edges))
    free_graph.push_back({free_edges[i].wu,
                          free_edges[i].wv,
                          free_labels[2 * i],
                          free_labels[2 * i + 1],
                          rank,
                          static_cast<int>(i)});

  std::vector<int> selected;
  Moose::MFEM::distributedSpanningForest(free_graph, num_vertices, comm, &selected);

  // 6. Map the selected edges back onto this rank's ND true dofs.
  std::vector<char> is_essential_tdof(pfes.GetTrueVSize(), 0);
  for (const auto i : make_range(essential_tdofs.Size()))
    is_essential_tdof[essential_tdofs[i]] = 1;

  // An edge chosen by the components at both of its ends is reported twice.
  std::sort(selected.begin(), selected.end());
  selected.erase(std::unique(selected.begin(), selected.end()), selected.end());

  mfem::Array<int> tree_tdofs;
  tree_tdofs.Reserve(static_cast<int>(selected.size()));
  for (const auto index : selected)
  {
    const int ltdof =
        pfes.GetLocalTDofNumber(edgeLDof(pfes, free_edges[index].mesh_edge, edge_dofs));
    if (ltdof < 0 || is_essential_tdof[ltdof])
      continue; // already essential via the boundary condition
    tree_tdofs.Append(ltdof);
  }
  // Keep the ascending tdof order the caller's elimination path expects.
  tree_tdofs.Sort();

  return tree_tdofs;
}

} // namespace

namespace Moose::MFEM
{

const mfem::Array<int> &
TreeCotreeGauge::trueDofs(const MFEMEssentialConstraint & constraint,
                          mfem::ParFiniteElementSpace & pfes,
                          const mfem::Array<int> * essential_bdr_markers,
                          const mfem::Array<int> & gauge_block_attrs)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();

  // Only the lowest-order edge dofs are gauged, so a higher order space would keep
  // the gradient modes carried by its remaining dofs and stay singular. Checked
  // before anything below relies on the space being order 1.
  if (pfes.GetMaxElementOrder() > 1)
    constraint.mooseError(
        "The tree-cotree gauge fixes the lowest-order edge degrees of freedom only, so it "
        "requires a FIRST order H(curl) space; the space of variable '",
        constraint.getTrialVariableName(),
        "' is order ",
        pfes.GetMaxElementOrder(),
        ". The gradient modes carried by the higher-order degrees of freedom would be left "
        "in place and the system would stay singular.");

  // The gauge is a property of the mesh and the space alone, so it only has to be
  // rebuilt when one of them is refined. Rebuilding it every time the linear forms
  // are assembled would repeat a global graph computation on every time step.
  if (_mesh_sequence == pmesh.GetSequence() && _fespace_sequence == pfes.GetSequence())
    return _tdofs;

  // Boundary attribute marker for an essential (tangential Dirichlet) condition
  // already applied to this variable; all-zero when no such boundary is given.
  mfem::Array<int> ess_bdr(pmesh.bdr_attributes.Size() ? pmesh.bdr_attributes.Max() : 0);
  ess_bdr = 0;
  if (essential_bdr_markers)
    ess_bdr = *essential_bdr_markers;

  // True dofs of the solve space already fixed by that boundary condition.
  mfem::Array<int> essential_tdofs;
  pfes.GetEssentialTrueDofs(ess_bdr, essential_tdofs);

  // Edges of subdomains that are not gauged (empty gauge_block_attrs -> none).
  mfem::Array<int> edge_excluded = excludedEdgeMarker(pfes, gauge_block_attrs);

  _tdofs = buildTreeCotreeGaugeTDofs(pfes, ess_bdr, essential_tdofs, edge_excluded);

  _mesh_sequence = pmesh.GetSequence();
  _fespace_sequence = pfes.GetSequence();
  return _tdofs;
}
}

#endif
