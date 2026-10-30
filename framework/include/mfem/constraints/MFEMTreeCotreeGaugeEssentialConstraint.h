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
#include "MFEMTreeCotreeGauge.h"

/**
 * Removes the gradient null space of an H(curl) (Nedelec) variable by strongly
 * fixing the edge degrees of freedom lying on a spanning tree of the mesh graph
 * to zero (a tree-cotree gauge). In a higher order space, a selection of the
 * degrees of freedom of every other gauged edge, face and element is also fixed,
 * removing the gradients of the higher order H1 functions. This makes curl-curl
 * systems with no (or only partial) mass regularization solvable.
 *
 * Constrained edges are seeded into the spanning forest so the interior gauge
 * stays compatible with the constraints rather than over-constraining them.
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
 * Above SECOND order the mesh must have no triangular faces, on which the
 * selection could not be made independently of the partitioning.
 */
class MFEMTreeCotreeGaugeEssentialConstraint : public MFEMEssentialConstraint
{
public:
  static InputParameters validParams();

  MFEMTreeCotreeGaugeEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list) override;

  /// The gauge seeds its spanning forest with every edge already fixed, so it must
  /// see the dofs fixed by all other essential conditions on the variable.
  bool appliedAfterOtherConstraints() const override { return true; }

protected:
  /// Selects and caches the gauged true dofs.
  Moose::MFEM::TreeCotreeGauge _gauge;
};

#endif
