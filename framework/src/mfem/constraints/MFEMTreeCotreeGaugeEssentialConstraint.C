//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTreeCotreeGaugeEssentialConstraint.h"

registerMooseObject("MooseApp", MFEMTreeCotreeGaugeEssentialConstraint);

InputParameters
MFEMTreeCotreeGaugeEssentialConstraint::validParams()
{
  InputParameters params = MFEMEssentialConstraint::validParams();
  params.addClassDescription("Strongly constrains a scalar variable in the specified domain.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "0.", "The coefficient setting the values on the essential boundary");
  return params;
}

MFEMTreeCotreeGaugeEssentialConstraint::MFEMTreeCotreeGaugeEssentialConstraint(
    const InputParameters & parameters)
  : MFEMEssentialConstraint(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

void
MFEMTreeCotreeGaugeEssentialConstraint::ApplyConstraint(mfem::ParGridFunction & gridfunc,
                                                        mfem::Array<int> & ess_tdof_list)
{
  mfem::Array<int> vdofs;
  mfem::Vector vals;
  mfem::DofTransformation doftrans;
  mfem::FiniteElementSpace * fes = gridfunc.FESpace();
  const mfem::Array<int> & domain_attrs = getSubdomainAttributes();

  for (const auto attribute : domain_attrs)
  {
    GetSubdomainTrueDofs(gridfunc, attribute, ess_tdof_list);
    for (int i = 0; i < fes->GetNE(); i++)
    {
      if (fes->GetAttribute(i) != attribute)
        continue;

      fes->GetElementVDofs(i, vdofs, doftrans);
      vals.SetSize(vdofs.Size());
      fes->GetFE(i)->Project(_coef, *fes->GetElementTransformation(i), vals);
      doftrans.TransformPrimal(vals);
      gridfunc.SetSubVector(vdofs, vals);
    }
  }
}
// Returns the list of true-dof indices (in the numbering of gf's
// ParFiniteElementSpace) that belong to elements carrying mesh
// attribute `attr`. Correct in parallel: dofs shared across an
// inter-processor boundary that coincides with the subdomain boundary
// are picked up even on ranks whose own elements don't have `attr`.
void
MFEMTreeCotreeGaugeEssentialConstraint::GetSubdomainTrueDofs(const mfem::ParGridFunction & gf,
                                                             int attr,
                                                             mfem::Array<int> & ess_tdofs)
{
  mfem::ParFiniteElementSpace * fespace = gf.ParFESpace();
  mfem::ParMesh * pmesh = fespace->GetParMesh();

  // 1. Local marker over L-dofs (vdofs): 1 if the dof touches at
  //    least one *local* element with the requested attribute.
  mfem::Array<int> dof_marker(fespace->GetVSize());
  dof_marker = 0;

  mfem::Array<int> vdofs;
  for (int e = 0; e < pmesh->GetNE(); e++)
  {
    if (pmesh->GetAttribute(e) == attr)
    {
      fespace->GetElementVDofs(e, vdofs);
      for (int i = 0; i < vdofs.Size(); i++)
      {
        // Undo the sign encoding used for vector-valued (ND/RT) dofs.
        int j = (vdofs[i] >= 0) ? vdofs[i] : -1 - vdofs[i];
        dof_marker[j] = 1;
      }
    }
  }

  // 2. Synchronize the marker across processors sharing a dof: this
  //    does a boolean-OR reduce + broadcast over the dof groups, so a
  //    shared dof ends up marked on every rank that touches it, even
  //    if that rank's own elements don't carry `attr`.
  fespace->Synchronize(dof_marker);

  // 3. Convert the (now-consistent) L-dof marker to a T-dof marker,
  //    exactly as GetEssentialTrueDofs does, then list the indices.
  mfem::Array<int> true_dof_marker;
  fespace->GetRestrictionMatrix()->BooleanMult(dof_marker, true_dof_marker);

  mfem::FiniteElementSpace::MarkerToList(true_dof_marker, ess_tdofs);
}

// ---------------------------------------------------------------------
// Minimal union-find (disjoint set) helper used to grow the spanning
// forest. Operates on dense 0-based global vertex ids.
// ---------------------------------------------------------------------
class UnionFind
{
public:
  explicit UnionFind(long long n) : parent(n), rnk(n, 0)
  {
    for (long long i = 0; i < n; i++)
    {
      parent[i] = i;
    }
  }

  long long Find(long long x)
  {
    while (parent[x] != x)
    {
      parent[x] = parent[parent[x]];
      x = parent[x];
    }
    return x;
  }

  // Returns true if a and b were in different components, i.e. the edge
  // between them closes no cycle and can join the spanning forest.
  bool Union(long long a, long long b)
  {
    a = Find(a);
    b = Find(b);
    if (a == b)
    {
      return false;
    }
    if (rnk[a] < rnk[b])
    {
      std::swap(a, b);
    }
    parent[b] = a;
    if (rnk[a] == rnk[b])
    {
      rnk[a]++;
    }
    return true;
  }

private:
  std::vector<long long> parent, rnk;
};

// ---------------------------------------------------------------------
// Build the *additional* essential true dofs coming from the interior
// tree-cotree gauge. Edges already fixed by the PEC boundary condition
// (listed in pec_tdofs) are used to seed the spanning forest so that the
// gauge is compatible with them, but are not duplicated in the return
// value.
//
//   pmesh      - the parallel mesh (its 1-skeleton is the graph)
//   fespace    - the actual (possibly high order) ND space being solved on
//   ess_bdr    - boundary attribute marker used for the PEC condition
//   pec_tdofs  - true dofs already made essential by ess_bdr
//
// Returns the list of additional true dofs (interior tree edges) that
// must be added to the essential dof list to remove the residual
// interior (order-1) gradient null space.
// ---------------------------------------------------------------------
mfem::Array<int>
BuildTreeCotreeInteriorTDofs(mfem::ParMesh & pmesh,
                             mfem::ParFiniteElementSpace & fespace,
                             const mfem::Array<int> & ess_bdr,
                             const mfem::Array<int> & pec_tdofs)
{
  MPI_Comm comm = pmesh.GetComm();
  int num_procs, myid;
  MPI_Comm_size(comm, &num_procs);
  MPI_Comm_rank(comm, &myid);
  const int dim = pmesh.Dimension();

  // Throwaway order-1 spaces, used only to obtain a globally unique,
  // densely-numbered id for every mesh vertex and every mesh edge, and to
  // know which of them this rank owns. This works regardless of the order
  // of the actual solve space, because ownership of a given mesh edge's
  // dofs is determined by the ParMesh group topology, not by the FE
  // space's polynomial order.
  mfem::H1_FECollection h1fec(1, dim);
  mfem::ParFiniteElementSpace h1fes(&pmesh, &h1fec);

  mfem::ND_FECollection nd1fec(1, dim);
  mfem::ParFiniteElementSpace nd1fes(&pmesh, &nd1fec);

  // Mark which ND1 local dofs (== mesh edges) are already essential via
  // the PEC boundary condition.
  mfem::Array<int> ess_marker;
  nd1fes.GetEssentialVDofs(ess_bdr, ess_marker);

  mfem::Array<HYPRE_BigInt> vtx_gid(pmesh.GetNV());
  for (int v = 0; v < pmesh.GetNV(); v++)
  {
    vtx_gid[v] = h1fes.GetGlobalTDofNumber(v);
  }

  // Collect this rank's *owned* edges as (gid, v0, v1, is_bdr). Only
  // owned edges are collected so each edge of the parallel mesh is
  // reported exactly once, by exactly one rank.
  struct EdgeRec
  {
    long long gid, v0, v1;
    int is_bdr;
  };
  std::vector<EdgeRec> owned;
  mfem::Array<int> dofs1, vert;

  for (int e = 0; e < pmesh.GetNEdges(); e++)
  {
    nd1fes.GetEdgeDofs(e, dofs1);
    int ldof = (dofs1[0] >= 0) ? dofs1[0] : (-1 - dofs1[0]);
    int ltdof = nd1fes.GetLocalTDofNumber(ldof);
    if (ltdof < 0)
    {
      continue;
    } // not owned by this rank

    pmesh.GetEdgeVertices(e, vert);
    EdgeRec rec;
    rec.gid = (long long)nd1fes.GetGlobalTDofNumber(ldof);
    rec.v0 = (long long)vtx_gid[vert[0]];
    rec.v1 = (long long)vtx_gid[vert[1]];
    rec.is_bdr = ess_marker[ldof] ? 1 : 0;
    owned.push_back(rec);
  }

  // Allgatherv the (small, topology-only) edge list to every rank. This
  // is cheap relative to the assembled matrix: a few integers per mesh
  // edge, gathered once. For extreme edge counts, replace this with a
  // genuinely distributed spanning-tree construction (see the
  // domain-decomposed tree-cotree literature referenced above).
  int local_n = (int)owned.size();
  std::vector<int> counts(num_procs), displs(num_procs);
  MPI_Allgather(&local_n, 1, MPI_INT, counts.data(), 1, MPI_INT, comm);
  int total_n = 0;
  for (int p = 0; p < num_procs; p++)
  {
    displs[p] = total_n;
    total_n += counts[p];
  }

  std::vector<long long> local_flat(4LL * local_n);
  for (int i = 0; i < local_n; i++)
  {
    local_flat[4 * i + 0] = owned[i].gid;
    local_flat[4 * i + 1] = owned[i].v0;
    local_flat[4 * i + 2] = owned[i].v1;
    local_flat[4 * i + 3] = owned[i].is_bdr;
  }

  std::vector<int> counts4(num_procs), displs4(num_procs);
  for (int p = 0; p < num_procs; p++)
  {
    counts4[p] = 4 * counts[p];
    displs4[p] = 4 * displs[p];
  }

  std::vector<long long> all_flat(4LL * total_n);
  MPI_Allgatherv(local_flat.data(),
                 4 * local_n,
                 MPI_LONG_LONG,
                 all_flat.data(),
                 counts4.data(),
                 displs4.data(),
                 MPI_LONG_LONG,
                 comm);

  std::vector<EdgeRec> all_edges(total_n);
  for (int i = 0; i < total_n; i++)
  {
    all_edges[i].gid = all_flat[4 * i + 0];
    all_edges[i].v0 = all_flat[4 * i + 1];
    all_edges[i].v1 = all_flat[4 * i + 2];
    all_edges[i].is_bdr = (int)all_flat[4 * i + 3];
  }
  // Sort by gid so every rank processes edges in the same order and
  // therefore builds an *identical* spanning forest, without needing a
  // separate broadcast step after this point.
  std::sort(all_edges.begin(),
            all_edges.end(),
            [](const EdgeRec & a, const EdgeRec & b) { return a.gid < b.gid; });

  // Build the spanning forest. Pass 1 unions vertices already tied
  // together by a PEC (Dirichlet) edge, without re-gauging them (they are
  // already fixed to zero by the boundary condition). Pass 2 grows the
  // tree over the remaining interior edges; an interior edge that closes
  // a cycle is a cotree edge and is left as a free unknown.
  long long global_nv = h1fes.GlobalTrueVSize();
  long long global_ne = nd1fes.GlobalTrueVSize();
  UnionFind uf(global_nv);
  std::vector<char> is_tree(global_ne, 0);

  for (const auto & e : all_edges)
  {
    if (e.is_bdr)
    {
      uf.Union(e.v0, e.v1);
    }
  }
  for (const auto & e : all_edges)
  {
    if (!e.is_bdr && uf.Union(e.v0, e.v1))
    {
      is_tree[e.gid] = 1;
    }
  }

  // Map the interior tree edges back onto local true dofs of the actual
  // (solve) finite element space, skipping anything already essential via
  // the PEC boundary.
  std::vector<char> is_pec(fespace.GetTrueVSize(), 0);
  for (int i = 0; i < pec_tdofs.Size(); i++)
  {
    is_pec[pec_tdofs[i]] = 1;
  }

  mfem::Array<int> tree_tdofs;
  mfem::Array<int> dofsP;
  for (int e = 0; e < pmesh.GetNEdges(); e++)
  {
    nd1fes.GetEdgeDofs(e, dofs1);
    int ldof1 = (dofs1[0] >= 0) ? dofs1[0] : (-1 - dofs1[0]);
    int ltdof1 = nd1fes.GetLocalTDofNumber(ldof1);
    if (ltdof1 < 0)
    {
      continue;
    } // not owned here
    long long gid = (long long)nd1fes.GetGlobalTDofNumber(ldof1);
    if (!is_tree[gid])
    {
      continue;
    } // cotree -> stays free

    fespace.GetEdgeDofs(e, dofsP);
    int ldofP = (dofsP[0] >= 0) ? dofsP[0] : (-1 - dofsP[0]);
    int ltdofP = fespace.GetLocalTDofNumber(ldofP);
    if (ltdofP < 0)
    {
      continue;
    } // ownership-consistency guard
    if (is_pec[ltdofP])
    {
      continue;
    } // already essential (PEC)

    tree_tdofs.Append(ltdofP);
  }

  return tree_tdofs;
}

#endif
