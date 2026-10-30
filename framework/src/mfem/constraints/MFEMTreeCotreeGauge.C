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
#include "MFEMConstraint.h"
#include "MooseError.h"
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
  explicit UnionFind(int n);

  int find(int x);

  // Returns true if a and b were in different components, i.e. the edge
  // between them closes no cycle and can join the spanning forest.
  bool join(int a, int b);

private:
  std::vector<int> _parent, _rank;
};

UnionFind::UnionFind(int n) : _parent(n), _rank(n, 0)
{
  std::iota(_parent.begin(), _parent.end(), 0);
}

int
UnionFind::find(int x)
{
  while (_parent[x] != x)
  {
    _parent[x] = _parent[_parent[x]];
    x = _parent[x];
  }
  return x;
}

bool
UnionFind::join(int a, int b)
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

// A vertex location, used as a partition-independent vertex identity.
using Coord3 = std::array<mfem::real_t, 3>;

// Local dof index with the sign MFEM uses to record an orientation flip removed.
int
decodeDof(int dof)
{
  return (dof >= 0) ? dof : -1 - dof;
}

// First dof of parallel mesh edge e, sign-decoded; the edge's dofs are left in
// dofs. Called once per mesh edge in each pass below, so the caller supplies the
// dof scratch space rather than having it reallocated for every edge.
int
edgeLDof(mfem::ParFiniteElementSpace & pfes, int e, mfem::Array<int> & dofs)
{
  pfes.GetEdgeDofs(e, dofs);
  return decodeDof(dofs[0]);
}

Coord3
vertexCoord(const mfem::ParMesh & pmesh, int v)
{
  // GetNode rather than GetVertex: on a curved (higher order geometry) mesh the
  // authoritative coordinates live in the mesh Nodes grid function, and the
  // vertices array is rebuilt per rank during partitioning, so its values are not
  // bit-identical between ranks that share a vertex. Keying on it there would give
  // one vertex several identities and a partition-dependent forest. GetNode reads
  // the Nodes grid function when the mesh is curved and falls back to the vertices
  // array when it is not. Indexing Nodes by vertex number relies on the vertex dofs
  // of a continuous (H1) nodal space being numbered first; TreeCotreeGauge::trueDofs
  // rejects discontinuous nodal spaces. Only spaceDim components are written, so the
  // unused trailing components keep the zero they are initialised with.
  Coord3 p{0.0, 0.0, 0.0};
  pmesh.GetNode(v, p.data());
  return p;
}

// Per-local-dof marker: 1 if the dof belongs to an element whose attribute is NOT
// set in the attribute marker array gauge_markers, i.e. a dof of a region the gauge
// must not reach because some other term of the weak form already removes the
// gradient null space there. Edges carrying such dofs seed the spanning forest, and
// no dof of such an edge, face or element is ever gauged. Dofs of a gauged region
// that merely touch such a region are NOT flagged, so the gauge still reaches the
// vertices next to it. Consistent across ranks for shared dofs.
mfem::Array<int>
excludedDofMarker(mfem::ParFiniteElementSpace & pfes, const mfem::Array<int> & gauge_markers)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();
  mfem::Array<int> marker(pfes.GetVSize());
  marker = 0;
  mfem::Array<int> edofs;
  for (const auto el : libMesh::make_range(pmesh.GetNE()))
    if (!gauge_markers[pmesh.GetAttribute(el) - 1])
    {
      pfes.GetElementDofs(el, edofs);
      for (const auto d : edofs)
        marker[decodeDof(d)] = 1;
    }
  pfes.Synchronize(marker); // boolean-OR over shared dofs
  return marker;
}

// ---------------------------------------------------------------------
// The gradient of an order p H1 function that vanishes on the boundary of a
// mesh entity (an edge, face or element "bubble") has no tangential trace on
// that boundary, so its ND interpolant lives entirely in the ND dofs that belong
// to the entity itself. This selects, on the reference entity, as many of those
// dofs as there are H1 bubbles, such that the discrete gradient restricted to the
// selected dofs and the bubbles is nonsingular: zero values on the selected dofs
// then admit no bubble gradient but zero.
//
// Rows of the discrete gradient block are chosen greedily by Gram-Schmidt with
// pivoting on the residual norm. Returns the indices of the selected dofs among
// the entity's own ND dofs, in ascending order; empty when the entity carries no
// H1 bubbles.
// ---------------------------------------------------------------------
std::vector<int>
referenceGaugedDofs(const mfem::FiniteElementCollection & nd_fec,
                    const mfem::FiniteElementCollection & h1_fec,
                    mfem::Geometry::Type geom,
                    int order)
{
  const int num_bubbles = h1_fec.GetNumDof(geom, order);
  std::vector<int> selected;
  if (num_bubbles == 0)
    return selected;

  const mfem::FiniteElement & nd_fe = *nd_fec.FiniteElementForGeometry(geom);
  const mfem::FiniteElement & h1_fe = *h1_fec.FiniteElementForGeometry(geom);
  mfem::IsoparametricTransformation identity;
  identity.SetIdentityTransformation(geom);
  mfem::DenseMatrix grad;
  nd_fe.ProjectGrad(h1_fe, identity, grad);

  // An entity's own dofs come last in MFEM's reference dof ordering, after those
  // of its vertices, edges and faces.
  const int num_rows = nd_fec.GetNumDof(geom, order);
  const int row0 = nd_fe.GetDof() - num_rows, col0 = h1_fe.GetDof() - num_bubbles;
  std::vector<mfem::Vector> rows(num_rows, mfem::Vector(num_bubbles));
  for (const auto r : libMesh::make_range(num_rows))
    for (const auto c : libMesh::make_range(num_bubbles))
      rows[r](c) = grad(row0 + r, col0 + c);

  // Symmetric reference elements produce rows whose residuals are equal in exact
  // arithmetic. Treating residuals within this relative tolerance as tied, and
  // taking the first of them, keeps the selection from depending on roundoff; it
  // is far above the roundoff of the few Gram-Schmidt steps involved and far below
  // any genuine difference between residuals.
  constexpr mfem::real_t tie_tolerance = 1e-8;
  std::vector<bool> available(num_rows, true);
  while (static_cast<int>(selected.size()) < num_bubbles)
  {
    mfem::real_t max_norm = 0;
    for (const auto r : libMesh::make_range(num_rows))
      if (available[r])
        max_norm = std::max(max_norm, rows[r].Norml2());

    int pick = 0;
    while (!available[pick] || rows[pick].Norml2() < (1 - tie_tolerance) * max_norm)
      ++pick;
    available[pick] = false;
    selected.push_back(pick);

    rows[pick] /= rows[pick].Norml2();
    for (const auto r : libMesh::make_range(num_rows))
      if (available[r])
        rows[r].Add(-(rows[r] * rows[pick]), rows[pick]);
  }

  std::sort(selected.begin(), selected.end());
  return selected;
}

// MFEM orientation, in the sense of Mesh::GetQuadOrientation, of a frame for
// quadrilateral face f fixed by its vertex coordinates alone: it starts at the
// lexicographically lowest vertex and heads to the lower of that vertex's two
// neighbours. The face's own vertex ordering, fv, comes from the local face
// numbering and can differ between partitions; this frame cannot.
int
canonicalQuadOrientation(const mfem::ParMesh & pmesh, const mfem::Array<int> & fv)
{
  std::array<Coord3, 4> coords;
  for (const auto i : libMesh::make_range(4))
    coords[i] = vertexCoord(pmesh, fv[i]);
  const int first = std::min_element(coords.begin(), coords.end()) - coords.begin();
  const int step = (coords[(first + 1) % 4] < coords[(first + 3) % 4]) ? 1 : 3;
  std::array<int, 4> canonical;
  for (const auto i : libMesh::make_range(4))
    canonical[i] = fv[(first + i * step) % 4];

  // Mesh::GetQuadOrientation(base, test) is protected. It returns the entry o of
  // the orientation table for which test[Orient[o][j]] == base[j] for every j.
  using Square = mfem::Geometry::Constants<mfem::Geometry::SQUARE>;
  for (const auto o : libMesh::make_range(Square::NumOrient))
  {
    std::array<int, 4> permuted;
    for (const auto j : libMesh::make_range(4))
      permuted[j] = canonical[Square::Orient[o][j]];
    if (std::equal(permuted.begin(), permuted.end(), fv.begin()))
      return o;
  }
  mooseError("No orientation maps the canonical frame of a quadrilateral face onto its vertices.");
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
// The coordinates come from the mesh nodes (see vertexCoord), which are
// bit-identical on every rank sharing a vertex and so give a
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
  for (const auto i : libMesh::index_range(coords))
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
  // global sorted position, which no choice of splitters can change. The number
  // of samples per rank trades bucket balance against the size of the sample every
  // rank gathers (oversample * nprocs coordinates); 32 keeps that gather small
  // while giving each splitter a few dozen candidates to be chosen from.
  constexpr std::size_t oversample = 32;
  const std::size_t num_samples = std::min(oversample, distinct.size());
  std::vector<Coord3> sample;
  for (const auto i : libMesh::make_range(num_samples))
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
  for (const auto p : libMesh::make_range(nprocs - 1))
    if (!all_samples.empty())
      splitters.push_back(all_samples[(p + 1) * all_samples.size() / nprocs]);
  auto bucketOf = [&](const Coord3 & c)
  {
    return static_cast<int>(std::upper_bound(splitters.begin(), splitters.end(), c) -
                            splitters.begin());
  };

  // Send each distinct local coordinate to the rank owning its bucket.
  std::vector<CoordQuery> outgoing(distinct.size());
  for (const auto i : libMesh::index_range(distinct))
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
  for (const auto i : libMesh::index_range(coords))
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
// fixed by essential boundary conditions or other essential constraints.
//
// The gradient null space of an order p ND space is the gradient of the order
// p H1 space, whose dofs sit on vertices and on the interiors of edges, faces
// and elements. The gauge removes it one entity dimension at a time. Fixing
// every dof of each edge of a spanning forest makes the potential constant
// along the forest, so it vanishes at every vertex. Fixing, on every free edge,
// face and element, a selection of its own dofs on which the gradients of the
// H1 bubbles there are linearly independent then leaves no bubble gradient
// either (see referenceGaugedDofs). With an order 1 space only the forest edges
// carry fixed dofs.
//
// The forest is grown with a distributed Boruvka pass over the mesh
// 1-skeleton: no rank ever holds more than its own share of the edges. Edges
// are ordered by their canonical global endpoint ids, and because that order
// is a total order on distinct weights the minimum spanning forest is unique.
// The dofs selected on each edge and face are chosen in a frame fixed by the
// vertex coordinates, and element interiors keep the element's vertex ordering
// under partitioning. The gauge - and hence the solution - therefore does not
// depend on the number of MPI ranks or on how the mesh was partitioned.
//
//   pfes            - the ND space being solved on
//   essential_tdofs - true dofs already fixed by other essential conditions;
//                     edges whose dofs are all fixed seed the forest, and no
//                     entity whose dofs are all fixed is gauged
//   excluded        - per-local-dof marker of dofs the gauge must not touch;
//                     their edges seed the forest like the fixed edges, and no
//                     edge, face or element carrying them is gauged
// ---------------------------------------------------------------------
mfem::Array<int>
buildTreeCotreeGaugeTDofs(mfem::ParFiniteElementSpace & pfes,
                          const mfem::Array<int> & essential_tdofs,
                          const mfem::Array<int> & excluded)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();
  MPI_Comm comm = pmesh.GetComm();
  int nprocs, rank;
  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &rank);

  // Dofs already fixed by other essential conditions. Only owned entities are
  // considered below, and all dofs of an owned entity are owned by this rank, so
  // marking this rank's true dofs is enough.
  std::vector<char> is_essential_tdof(pfes.GetTrueVSize(), 0);
  for (const auto tdof : essential_tdofs)
    is_essential_tdof[tdof] = 1;
  auto allFixed = [&](const mfem::Array<int> & dofs)
  {
    return std::all_of(dofs.begin(),
                       dofs.end(),
                       [&](int d)
                       { return is_essential_tdof[pfes.GetLocalTDofNumber(decodeDof(d))] != 0; });
  };

  // The dofs on each reference entity that remove the gradients of its H1 bubbles.
  const mfem::FiniteElementCollection & nd_fec = *pfes.FEColl();
  const int order = nd_fec.GetOrder();
  const mfem::H1_FECollection h1_fec(order, pmesh.Dimension());
  const auto edge_gauged = referenceGaugedDofs(nd_fec, h1_fec, mfem::Geometry::SEGMENT, order);

  // 1. This rank's owned edges. Each mesh edge is owned by exactly one rank, so
  //    the union over ranks is the mesh 1-skeleton with no duplicates.
  std::vector<OwnedEdge> owned_edges;
  mfem::Array<int> ev, edge_dofs;
  for (const auto e : libMesh::make_range(pmesh.GetNEdges()))
  {
    const int ldof = edgeLDof(pfes, e, edge_dofs);
    if (pfes.GetLocalTDofNumber(ldof) < 0)
      continue; // owned by another rank; it will contribute this edge

    pmesh.GetEdgeVertices(e, ev);
    owned_edges.push_back({vertexCoord(pmesh, ev[0]),
                           vertexCoord(pmesh, ev[1]),
                           e,
                           excluded[ldof] != 0 || allFixed(edge_dofs)});
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
  // Every free edge is gauged, including those filtered out below.
  std::vector<int> free_mesh_edges;
  for (const auto i : libMesh::index_range(owned_edges))
  {
    const std::int64_t a = numbering.global_id[2 * i], b = numbering.global_id[2 * i + 1];
    const int la = numbering.local_index[2 * i], lb = numbering.local_index[2 * i + 1];
    // Order both representations of the endpoints the same way.
    const CandidateEdge c = (a < b) ? CandidateEdge{a, b, la, lb, owned_edges[i].mesh_edge}
                                    : CandidateEdge{b, a, lb, la, owned_edges[i].mesh_edge};
    (owned_edges[i].seed ? seed_edges : free_edges).push_back(c);
    if (!owned_edges[i].seed)
      free_mesh_edges.push_back(owned_edges[i].mesh_edge);
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
  for (const auto i : libMesh::index_range(seed_edges))
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
  //    cycle and is left out of the forest (a cotree edge).
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
  for (const auto i : libMesh::index_range(free_edges))
    free_graph.push_back({free_edges[i].wu,
                          free_edges[i].wv,
                          free_labels[2 * i],
                          free_labels[2 * i + 1],
                          rank,
                          static_cast<int>(i)});

  std::vector<int> selected;
  Moose::MFEM::distributedSpanningForest(free_graph, num_vertices, comm, &selected);

  // Every selected edge is owned by this rank. An edge chosen by the components at
  // both of its ends is reported twice, which marking absorbs.
  std::vector<char> in_forest(pmesh.GetNEdges(), 0);
  for (const auto index : selected)
    in_forest[free_edges[index].mesh_edge] = 1;

  mfem::Array<int> gauged_tdofs;
  auto gauge = [&](int ldof) { gauged_tdofs.Append(pfes.GetLocalTDofNumber(decodeDof(ldof))); };

  // 6. Fix every dof of a forest edge, and the bubble-removing dofs of any other
  //    free edge. MFEM orders the dofs of an edge from its lower-numbered local
  //    vertex; the selection is made from the edge's lexicographically lower
  //    endpoint instead, so it does not depend on the local numbering.
  for (const auto e : free_mesh_edges)
  {
    pfes.GetEdgeDofs(e, edge_dofs);
    if (in_forest[e])
    {
      for (const auto d : edge_dofs)
        gauge(d);
      continue;
    }
    pmesh.GetEdgeVertices(e, ev);
    const bool reversed =
        vertexCoord(pmesh, std::max(ev[0], ev[1])) < vertexCoord(pmesh, std::min(ev[0], ev[1]));
    for (const auto k : edge_gauged)
      gauge(edge_dofs[reversed ? edge_dofs.Size() - 1 - k : k]);
  }

  // 7. Fix the bubble-removing dofs of every owned free face of a 3D mesh, chosen
  //    in the frame of canonicalQuadOrientation. Triangular faces carry no bubbles
  //    at the orders TreeCotreeGauge::trueDofs accepts.
  const auto face_gauged = (pmesh.Dimension() == 3)
                               ? referenceGaugedDofs(nd_fec, h1_fec, mfem::Geometry::SQUARE, order)
                               : std::vector<int>();
  if (!face_gauged.empty())
  {
    mfem::Array<int> face_dofs, fv;
    for (const auto f : libMesh::make_range(pmesh.GetNFaces()))
    {
      if (pmesh.GetFaceGeometry(f) != mfem::Geometry::SQUARE)
        continue;
      pfes.GetFaceInteriorDofs(f, face_dofs);
      if (pfes.GetLocalTDofNumber(face_dofs[0]) < 0 || excluded[face_dofs[0]] ||
          allFixed(face_dofs))
        continue;

      pmesh.GetFaceVertices(f, fv);
      // Maps each dof of the canonical frame onto the face's own dofs.
      const int * canonical_to_face = nd_fec.DofOrderForOrientation(
          mfem::Geometry::SQUARE, canonicalQuadOrientation(pmesh, fv));
      for (const auto k : face_gauged)
        gauge(face_dofs[decodeDof(canonical_to_face[k])]);
    }
  }

  // 8. Fix the bubble-removing dofs of every free element. Element interior dofs
  //    are never shared, and their frame is the element's vertex ordering, which
  //    partitioning preserves.
  std::array<std::vector<int>, mfem::Geometry::NumGeom> interior_gauged;
  mfem::Array<mfem::Geometry::Type> element_geoms;
  pmesh.GetGeometries(pmesh.Dimension(), element_geoms);
  for (const auto geom : element_geoms)
    interior_gauged[geom] = referenceGaugedDofs(nd_fec, h1_fec, geom, order);

  mfem::Array<int> interior_dofs;
  for (const auto el : libMesh::make_range(pmesh.GetNE()))
  {
    const auto & gauged = interior_gauged[pmesh.GetElementGeometry(el)];
    if (gauged.empty())
      continue;
    pfes.GetElementInteriorDofs(el, interior_dofs);
    if (excluded[interior_dofs[0]] || allFixed(interior_dofs))
      continue;
    for (const auto k : gauged)
      gauge(interior_dofs[k]);
  }

  // Keep the ascending tdof order the caller's elimination path expects.
  gauged_tdofs.Sort();

  return gauged_tdofs;
}

} // namespace

namespace Moose::MFEM
{

const mfem::Array<int> &
TreeCotreeGauge::trueDofs(const MFEMConstraint & constraint,
                          mfem::ParFiniteElementSpace & pfes,
                          const mfem::Array<int> & essential_tdofs,
                          const mfem::Array<int> & gauge_block_markers)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();

  // The graph is built from the mesh edges and the lowest-order dof on each, which
  // is only meaningful for a Nedelec space: an H1 space would hand back vertex dofs
  // and an RT space none at all.
  if (!dynamic_cast<const mfem::ND_FECollection *>(pfes.FEColl()))
    constraint.paramError("variable",
                          "The tree-cotree gauge requires an H(curl) (Nedelec) variable, but '",
                          constraint.getTrialVariableName(),
                          "' uses the finite element collection '",
                          pfes.FEColl()->Name(),
                          "'.");

  // From THIRD order on, a triangular face carries H1 bubbles whose gradients the
  // gauge must remove with a selection of the face's dofs. MFEM relates the dofs of
  // a triangular face in different frames by a 2x2 transformation per point, not a
  // permutation, so no selection is the same in every frame, and the frame of a
  // shared face depends on the partitioning. Checked on every rank, since only some
  // ranks may hold such faces.
  int has_triangles = pmesh.Dimension() == 3 && pmesh.HasGeometry(mfem::Geometry::TRIANGLE);
  MPI_Allreduce(MPI_IN_PLACE, &has_triangles, 1, MPI_INT, MPI_LOR, pmesh.GetComm());
  if (pfes.GetMaxElementOrder() > 2 && has_triangles)
    constraint.paramError(
        "variable",
        "The tree-cotree gauge supports H(curl) spaces above SECOND order only on meshes "
        "without triangular faces, since the gauge on such a face could not be chosen "
        "independently of the mesh partitioning; the space of variable '",
        constraint.getTrialVariableName(),
        "' is order ",
        pfes.GetMaxElementOrder(),
        ".");

  // Vertices are identified by their coordinates, read from the mesh nodes. A
  // discontinuous nodal space, as carried by a periodic mesh, has no single node per
  // vertex, and periodically identified vertices have no single coordinate.
  if (pmesh.GetNodes() && pmesh.GetNodalFESpace()->IsDGSpace())
    constraint.mooseError("The tree-cotree gauge identifies mesh vertices by their coordinates, "
                          "which is not possible on a mesh with discontinuous nodes, such as a "
                          "periodic mesh.");

  // The gauge is a property of the mesh, the space and the dofs already fixed
  // alone, so it only has to be rebuilt when one of them changes. Rebuilding it
  // every time the linear forms are assembled would repeat a global graph
  // computation on every time step.
  if (_fespace_sequence == pfes.GetSequence() && _essential_tdofs == essential_tdofs)
    return _tdofs;

  // Dofs of subdomains that are not gauged.
  mfem::Array<int> excluded = excludedDofMarker(pfes, gauge_block_markers);

  _tdofs = buildTreeCotreeGaugeTDofs(pfes, essential_tdofs, excluded);

  _fespace_sequence = pfes.GetSequence();
  _essential_tdofs = essential_tdofs;
  return _tdofs;
}
}

#endif
