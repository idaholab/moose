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

/**
 * Shared implementation of the tree-cotree gauge for an H(curl) (Nedelec)
 * variable, used by both the real (MFEMTreeCotreeGaugeEssentialConstraint) and
 * complex (MFEMComplexTreeCotreeGaugeEssentialConstraint) constraints. Only the
 * per-component grid-function bookkeeping differs between the two; the choice of
 * degrees of freedom to fix is identical and lives here.
 */
class TreeCotreeGaugeBuilder
{
protected:
  /**
   * This rank's ND true-dof indices that the tree-cotree gauge must strongly set
   * to zero, in addition to those already fixed by the tangential Dirichlet
   * ("PEC") boundary condition.
   *
   * The mesh 1-skeleton is gathered onto every rank with edges keyed on their
   * endpoint coordinates (bit-identical across ranks), so one canonical seeded
   * spanning forest is grown identically on every rank; the result is
   * independent of the MPI partitioning.
   *
   * @param pfes              the (possibly high order) ND space being solved on
   * @param pec_bdr_markers   boundary attribute marker for the PEC condition, or
   *                          nullptr when there is no such boundary
   * @param gauge_block_attrs subdomain attributes to gauge; edges of the
   *                          complementary subdomains seed the forest but are
   *                          never gauged. Empty gauges the whole mesh.
   */
  static mfem::Array<int> gaugeTrueDofs(mfem::ParFiniteElementSpace & pfes,
                                        const mfem::Array<int> * pec_bdr_markers,
                                        const mfem::Array<int> & gauge_block_attrs);
};

#endif
