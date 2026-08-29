//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{
/**
 * Shared implementation of the tree-cotree gauge for an H(curl) (Nedelec)
 * variable, used by both the real (MFEMTreeCotreeGaugeEssentialConstraint) and
 * complex (MFEMComplexTreeCotreeGaugeEssentialConstraint) constraints. Only the
 * per-component grid-function bookkeeping differs between the two; the choice of
 * degrees of freedom to fix is identical and lives here.
 *
 * The gauge depends only on the mesh and the finite element space, so the result
 * is cached and only rebuilt when either is refined. Held by value as a member of
 * the constraint that uses it.
 */
class TreeCotreeGauge
{
public:
  /**
   * This rank's ND true-dof indices that the tree-cotree gauge must strongly set
   * to zero, in addition to those already fixed by the tangential Dirichlet
   * ("PEC") boundary condition.
   *
   * The seeded spanning forest is grown by a distributed Boruvka pass, so no
   * rank holds more than its own share of the mesh graph. Edges are weighted by
   * their canonical global endpoint ids, which are derived from the (bit-identical
   * across ranks) endpoint coordinates; because those weights are distinct the
   * minimum spanning forest is unique and the result is independent of the MPI
   * partitioning.
   *
   * The returned reference stays valid until the next call on this object.
   *
   * @param pfes              the ND space being solved on. Only the lowest-order
   *                          (edge) dofs are gauged, so the space must be order 1
   * @param pec_bdr_markers   boundary attribute marker for the PEC condition, or
   *                          nullptr when there is no such boundary
   * @param gauge_block_attrs subdomain attributes to gauge; edges of the
   *                          complementary subdomains seed the forest but are
   *                          never gauged. Empty gauges the whole mesh.
   */
  const mfem::Array<int> & trueDofs(mfem::ParFiniteElementSpace & pfes,
                                    const mfem::Array<int> * pec_bdr_markers,
                                    const mfem::Array<int> & gauge_block_attrs);

private:
  /// Gauged true dofs of the most recent trueDofs() call.
  mfem::Array<int> _tdofs;
  /// Mesh and FE space sequence numbers _tdofs was built for. Both counters are
  /// bumped by MFEM on h- and p-refinement, so comparing them invalidates the
  /// cache exactly when the gauge could have changed. -1 marks "never built".
  long _mesh_sequence = -1;
  long _fespace_sequence = -1;
};
}

#endif
