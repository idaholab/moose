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

#include "MFEMEssentialConstraint.h"
#include "MFEMBoundaryRestrictable.h"
#include "MFEMTreeCotreeGauge.h"

/**
 * Removes the gradient null space of an H(curl) (Nedelec) variable by strongly
 * fixing the lowest-order edge degrees of freedom lying on a spanning tree of
 * the mesh graph to zero (a tree-cotree gauge). This makes curl-curl systems
 * with no (or only partial) mass regularization solvable.
 *
 * The 'boundary' parameter should list exactly the boundaries on which a
 * tangential Dirichlet condition is applied to the variable: those edges are
 * seeded into the spanning forest so the interior gauge stays compatible with
 * the boundary condition rather than over-constraining it.
 *
 * The 'block' parameter restricts the gauge to the given subdomains, for the
 * case where another term of the weak form already removes the null space
 * elsewhere. Edges of the excluded subdomains seed the forest but are not
 * gauged. An empty 'block' gauges the whole mesh.
 *
 * The spanning forest is grown by a distributed Boruvka pass under a canonical,
 * geometry-derived edge order, so the gauge is independent of the MPI
 * partitioning.
 *
 * Only the lowest-order edge dofs are gauged, so the variable's finite element
 * space must be order 1; a higher order space would keep the gradient modes
 * carried by its interior dofs and stay singular.
 */
class MFEMTreeCotreeGaugeEssentialConstraint : public MFEMEssentialConstraint,
                                               public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMTreeCotreeGaugeEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list) override;

protected:
  /// Selects and caches the gauged true dofs.
  Moose::MFEM::TreeCotreeGauge _gauge;
};

#endif
