//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "TreeCotreeGaugeBuilder.h"
#include "libmesh/int_range.h"

#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <tuple>

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
using EdgeKey = std::pair<int, int>;

// Lowest-order edge dof of parallel mesh edge e, sign-decoded.
int
edgeLDof(mfem::ParFiniteElementSpace & pfes, int e)
{
  mfem::Array<int> dofs;
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

// ---------------------------------------------------------------------
// Build the interior tree-cotree gauge as a list of this rank's ND true
// dofs that must be strongly set to zero, in addition to those already
// fixed by the PEC (tangential Dirichlet) boundary condition.
//
// The mesh 1-skeleton is gathered from every rank with edges keyed on
// their two endpoint coordinates. Vertex coordinates are copied verbatim
// from the serial mesh during partitioning, so they are bit-identical on
// every rank and give a partition-independent edge identity. A single
// canonical seeded spanning forest is then grown from that gathered list
// (identically on every rank) and mapped back onto this rank's dofs, so
// the gauge - and hence the solution - does not depend on the number of
// MPI ranks or on how the mesh was partitioned.
//
//   pfes           - the (possibly high order) ND space being solved on
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
  int num_procs;
  MPI_Comm_size(comm, &num_procs);

  // Lowest-order edge dofs fixed by the PEC boundary condition.
  mfem::Array<int> ess_vdof_marker;
  pfes.GetEssentialVDofs(ess_bdr, ess_vdof_marker);

  // 1. This rank's owned edges, flattened as
  //    [x0 y0 z0  x1 y1 z1  is_pec  is_excluded].
  constexpr int stride = 8;
  std::vector<mfem::real_t> local;
  mfem::Array<int> ev;
  for (const auto e : make_range(pmesh.GetNEdges()))
  {
    const int ldof = edgeLDof(pfes, e);
    if (pfes.GetLocalTDofNumber(ldof) < 0)
      continue; // owned by another rank; it will contribute this edge

    pmesh.GetEdgeVertices(e, ev);
    const Coord3 p0 = vertexCoord(pmesh, ev[0]);
    const Coord3 p1 = vertexCoord(pmesh, ev[1]);
    for (const auto d : make_range(3))
      local.push_back(p0[d]);
    for (const auto d : make_range(3))
      local.push_back(p1[d]);
    local.push_back(ess_vdof_marker[ldof] ? 1.0 : 0.0);
    local.push_back(edge_excluded[e] ? 1.0 : 0.0);
  }

  // 2. Gather every rank's edge list onto every rank.
  int local_count = static_cast<int>(local.size());
  std::vector<int> counts(num_procs), displs(num_procs);
  MPI_Allgather(&local_count, 1, MPI_INT, counts.data(), 1, MPI_INT, comm);
  int total = 0;
  for (const auto p : make_range(num_procs))
  {
    displs[p] = total;
    total += counts[p];
  }
  std::vector<mfem::real_t> all(total);
  MPI_Allgatherv(local.data(),
                 local_count,
                 MFEM_MPI_REAL_T,
                 all.data(),
                 counts.data(),
                 displs.data(),
                 MFEM_MPI_REAL_T,
                 comm);
  const int num_edges = total / stride;

  // 3. Assign every distinct endpoint coordinate a dense id in a canonical
  //    (coordinate-sorted) order, so the forest below is identical on every
  //    rank.
  auto coord0 = [&](int i) -> Coord3
  { return {all[stride * i + 0], all[stride * i + 1], all[stride * i + 2]}; };
  auto coord1 = [&](int i) -> Coord3
  { return {all[stride * i + 3], all[stride * i + 4], all[stride * i + 5]}; };

  std::map<Coord3, int> vertex_id;
  for (const auto i : make_range(num_edges))
  {
    vertex_id.emplace(coord0(i), 0);
    vertex_id.emplace(coord1(i), 0);
  }
  int next_id = 0;
  for (auto & [_, id] : vertex_id)
    id = next_id++;
  auto vid = [&](const Coord3 & p) { return libmesh_map_find(vertex_id, p); };

  // Canonical (v0 < v1) key for every gathered edge, with its "seed" flag: an
  // edge seeds the forest (and is never gauged) if it lies on the PEC boundary
  // or is an edge of the excluded (conductor) region.
  struct Edge
  {
    EdgeKey key;
    bool seed;
    bool operator<(const Edge & o) const { return std::tie(key, seed) < std::tie(o.key, o.seed); }
  };
  std::vector<Edge> edges(num_edges);
  for (const auto i : make_range(num_edges))
  {
    const int a = vid(coord0(i));
    const int b = vid(coord1(i));
    edges[i] = {EdgeKey{std::min(a, b), std::max(a, b)},
                all[stride * i + 6] != 0.0 || all[stride * i + 7] != 0.0};
  }
  // Deterministic processing order for pass 2.
  std::sort(edges.begin(), edges.end());

  // 4. Seeded spanning forest. Pass 1 unions the endpoints of every seed edge
  //    (PEC boundary or conductor), grounding those regions without gauging
  //    them. Pass 2 grows the tree over the remaining edges - including vacuum
  //    edges that touch the conductor surface - so every free vertex is
  //    reached; an edge that would close a cycle is a cotree edge, left free.
  UnionFind uf(next_id);
  for (const auto & e : edges)
    if (e.seed)
      uf.join(e.key.first, e.key.second);

  std::set<EdgeKey> tree_edges;
  for (const auto & e : edges)
    if (!e.seed && uf.join(e.key.first, e.key.second))
      tree_edges.insert(e.key);

  // 5. Map tree edges back onto this rank's ND true dofs.
  std::vector<char> is_pec_tdof(pfes.GetTrueVSize(), 0);
  for (const auto i : make_range(pec_tdofs.Size()))
    is_pec_tdof[pec_tdofs[i]] = 1;

  mfem::Array<int> tree_tdofs;
  for (const auto e : make_range(pmesh.GetNEdges()))
  {
    const int ldof = edgeLDof(pfes, e);
    const int ltdof = pfes.GetLocalTDofNumber(ldof);
    if (ltdof < 0)
      continue;

    pmesh.GetEdgeVertices(e, ev);
    const int a = vid(vertexCoord(pmesh, ev[0]));
    const int b = vid(vertexCoord(pmesh, ev[1]));
    if (!tree_edges.count(EdgeKey{std::min(a, b), std::max(a, b)}))
      continue;
    if (is_pec_tdof[ltdof])
      continue; // already essential via the PEC boundary
    tree_tdofs.Append(ltdof);
  }
  return tree_tdofs;
}

} // namespace

mfem::Array<int>
TreeCotreeGaugeBuilder::gaugeTrueDofs(mfem::ParFiniteElementSpace & pfes,
                                      const mfem::Array<int> * pec_bdr_markers,
                                      const mfem::Array<int> & gauge_block_attrs)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();

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

  return buildTreeCotreeGaugeTDofs(pfes, ess_bdr, pec_tdofs, edge_excluded);
}

#endif
