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
// NOT among gauge_attrs, i.e. an edge of a region the gauge must not reach (e.g.
// a conductor whose sigma * dA/dt term already fixes the gauge there). These
// edges seed the spanning forest but are never gauged. Vacuum edges that merely
// touch the conductor surface are NOT flagged, so the gauge still reaches the
// vacuum vertices next to the conductor. Consistent across ranks for shared
// edges. An empty gauge_attrs gauges the whole mesh and flags nothing.
mfem::Array<int>
excludedEdgeMarker(mfem::ParMesh & pmesh, const mfem::Array<int> & gauge_attrs)
{
  mfem::Array<int> excluded(pmesh.GetNEdges());
  excluded = 0;
  if (gauge_attrs.Size() == 0)
    return excluded;

  const int max_attr = pmesh.attributes.Size() ? pmesh.attributes.Max() : 0;
  std::vector<char> is_gauge_attr(max_attr + 1, 0);
  for (const auto a : gauge_attrs)
    if (a >= 1 && a <= max_attr)
      is_gauge_attr[a] = 1;

  // Throwaway order-1 Nedelec space: used only for its element/edge dof adjacency
  // and the group communicator (Synchronize), never for its dof numbering.
  mfem::ND_FECollection nd1fec(1, pmesh.Dimension());
  mfem::ParFiniteElementSpace nd1fes(&pmesh, &nd1fec);
  mfem::Array<int> marker(nd1fes.GetVSize());
  marker = 0;
  mfem::Array<int> edofs;
  for (const auto el : make_range(pmesh.GetNE()))
    if (!is_gauge_attr[pmesh.GetAttribute(el)])
    {
      nd1fes.GetElementDofs(el, edofs);
      for (const auto d : edofs)
        marker[d < 0 ? -1 - d : d] = 1;
    }
  nd1fes.Synchronize(marker); // boolean-OR over shared edges

  for (const auto e : make_range(pmesh.GetNEdges()))
  {
    nd1fes.GetEdgeDofs(e, edofs);
    excluded[e] = marker[edofs[0] < 0 ? -1 - edofs[0] : edofs[0]];
  }
  return excluded;
}

// One of this rank's owned mesh edges, keyed on its endpoint coordinates.
struct OwnedEdge
{
  Coord3 p0, p1;
  /// Index into the local ParMesh edge numbering, used to recover the ND dof.
  int mesh_edge;
  /// Seeds the forest (PEC boundary or excluded region) and is never gauged.
  bool seed;
};

// A candidate edge once its endpoints carry canonical global vertex ids.
struct CandidateEdge
{
  std::int64_t wu, wv; // canonical, wu < wv
  int mesh_edge;
};

// Round-trip record for looking up the canonical id of a coordinate.
struct CoordQuery
{
  Coord3 coord;
  std::int64_t id;
  int origin_rank, origin_index;
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
std::vector<std::int64_t>
canonicalVertexIds(const std::vector<Coord3> & coords, MPI_Comm comm, std::int64_t & num_vertices)
{
  int nprocs, rank;
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);

  std::vector<Coord3> distinct(coords);
  std::sort(distinct.begin(), distinct.end());
  distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());

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
  MPI_Allreduce(&owned_count, &num_vertices, 1, MPI_INT64_T, MPI_SUM, comm);

  for (auto & q : received)
    q.id = offset + (std::lower_bound(owned.begin(), owned.end(), q.coord) - owned.begin());

  std::vector<int> answer_counts;
  const auto answers = Moose::MFEM::groupByDestination(
      received, nprocs, [](const CoordQuery & q) { return q.origin_rank; }, answer_counts);

  std::vector<std::int64_t> distinct_ids(distinct.size());
  for (const auto & q : Moose::MFEM::allToAll(answers, answer_counts, comm))
    distinct_ids[q.origin_index] = q.id;

  // Finally translate the caller's (non-distinct) coordinate list.
  std::vector<std::int64_t> ids(coords.size());
  for (const auto i : index_range(coords))
    ids[i] = distinct_ids[std::lower_bound(distinct.begin(), distinct.end(), coords[i]) -
                          distinct.begin()];
  return ids;
}

// ---------------------------------------------------------------------
// Discard edges that close a cycle among this rank's own edges. By the cycle
// property such an edge is the heaviest on that cycle, so it cannot belong to
// the minimum spanning forest of the whole graph either, and dropping it here
// keeps it out of every subsequent communication round. Contracting the seed
// components later can only shorten those cycles, never break them, so the
// filter stays valid for the contracted graph too.
//
// Returns the surviving edges; @p local_ids is the sorted list of distinct
// global vertex ids appearing in @p candidates.
// ---------------------------------------------------------------------
std::vector<CandidateEdge>
filterLocalCycles(std::vector<CandidateEdge> candidates,
                  const std::vector<std::int64_t> & local_ids)
{
  // Kruskal in the canonical edge order, so the survivors are exactly those a
  // serial pass would still have to consider.
  std::sort(candidates.begin(),
            candidates.end(),
            [](const CandidateEdge & a, const CandidateEdge & b)
            { return (a.wu != b.wu) ? a.wu < b.wu : a.wv < b.wv; });

  auto index = [&](std::int64_t g)
  {
    return static_cast<int>(std::lower_bound(local_ids.begin(), local_ids.end(), g) -
                            local_ids.begin());
  };

  UnionFind uf(static_cast<int>(local_ids.size()));
  std::vector<CandidateEdge> kept;
  for (const auto & c : candidates)
    if (uf.join(index(c.wu), index(c.wv)))
      kept.push_back(c);
  return kept;
}

// ---------------------------------------------------------------------
// Build the interior tree-cotree gauge as a list of this rank's ND true
// dofs that must be strongly set to zero, in addition to those already
// fixed by the PEC (tangential Dirichlet) boundary condition.
//
// The forest is grown with a distributed Boruvka pass over the mesh
// 1-skeleton: no rank ever holds more than its own share of the edges. Edges
// are ordered by their canonical global endpoint ids, and because that order
// is a total order on distinct weights the minimum spanning forest is unique.
// The gauge - and hence the solution - therefore does not depend on the number
// of MPI ranks or on how the mesh was partitioned.
//
//   pfes           - the ND space being solved on
//   ess_bdr        - boundary attribute marker for the PEC condition
//   pec_tdofs      - true dofs already made essential by ess_bdr
//   edge_excluded  - per-local-edge marker of edges the gauge must not touch
//                    (edges of the conductor region); they seed the forest like
//                    the PEC boundary but are never gauged
// ---------------------------------------------------------------------
mfem::Array<int>
buildTreeCotreeGaugeTDofs(mfem::ParFiniteElementSpace & pfes,
                          const mfem::Array<int> & ess_bdr,
                          const mfem::Array<int> & pec_tdofs,
                          const mfem::Array<int> & edge_excluded)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();
  MPI_Comm comm = pmesh.GetComm();
  int nprocs, rank;
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);

  // Lowest-order edge dofs fixed by the PEC boundary condition.
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
  std::int64_t num_vertices = 0;
  const auto vertex_ids = canonicalVertexIds(endpoints, comm, num_vertices);

  std::vector<CandidateEdge> seed_edges, free_edges;
  for (const auto i : index_range(owned_edges))
  {
    const std::int64_t a = vertex_ids[2 * i], b = vertex_ids[2 * i + 1];
    const CandidateEdge c{std::min(a, b), std::max(a, b), owned_edges[i].mesh_edge};
    (owned_edges[i].seed ? seed_edges : free_edges).push_back(c);
  }

  std::vector<std::int64_t> local_ids(vertex_ids);
  std::sort(local_ids.begin(), local_ids.end());
  local_ids.erase(std::unique(local_ids.begin(), local_ids.end()), local_ids.end());

  // 3. Drop locally redundant edges before any communication.
  seed_edges = filterLocalCycles(std::move(seed_edges), local_ids);
  free_edges = filterLocalCycles(std::move(free_edges), local_ids);

  // 4. Ground the seeded regions: the connected components of the PEC boundary
  //    and of the excluded subdomains, which are fixed without being gauged.
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
  std::vector<char> is_pec_tdof(pfes.GetTrueVSize(), 0);
  for (const auto i : make_range(pec_tdofs.Size()))
    is_pec_tdof[pec_tdofs[i]] = 1;

  // An edge chosen by the components at both of its ends is reported twice.
  std::sort(selected.begin(), selected.end());
  selected.erase(std::unique(selected.begin(), selected.end()), selected.end());

  mfem::Array<int> tree_tdofs;
  for (const auto index : selected)
  {
    const int ltdof =
        pfes.GetLocalTDofNumber(edgeLDof(pfes, free_edges[index].mesh_edge, edge_dofs));
    if (ltdof < 0 || is_pec_tdof[ltdof])
      continue; // already essential via the PEC boundary
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
TreeCotreeGauge::trueDofs(mfem::ParFiniteElementSpace & pfes,
                          const mfem::Array<int> * pec_bdr_markers,
                          const mfem::Array<int> & gauge_block_attrs)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();

  // The gauge is a property of the mesh and the space alone, so it only has to be
  // rebuilt when one of them is refined. Rebuilding it every time the linear forms
  // are assembled would repeat an all-to-all gather of the mesh 1-skeleton on
  // every time step.
  if (_mesh_sequence == pmesh.GetSequence() && _fespace_sequence == pfes.GetSequence())
    return _tdofs;

  // Boundary attribute marker for the tangential Dirichlet ("PEC") condition on
  // this variable; all-zero when no such boundary is given.
  mfem::Array<int> ess_bdr(pmesh.bdr_attributes.Size() ? pmesh.bdr_attributes.Max() : 0);
  ess_bdr = 0;
  if (pec_bdr_markers)
    ess_bdr = *pec_bdr_markers;

  // True dofs of the solve space already fixed by the PEC boundary condition.
  mfem::Array<int> pec_tdofs;
  pfes.GetEssentialTrueDofs(ess_bdr, pec_tdofs);

  // Edges of subdomains that are not gauged (empty gauge_block_attrs -> none).
  mfem::Array<int> edge_excluded = excludedEdgeMarker(pmesh, gauge_block_attrs);

  _tdofs = buildTreeCotreeGaugeTDofs(pfes, ess_bdr, pec_tdofs, edge_excluded);
  _mesh_sequence = pmesh.GetSequence();
  _fespace_sequence = pfes.GetSequence();
  return _tdofs;
}
}

#endif
