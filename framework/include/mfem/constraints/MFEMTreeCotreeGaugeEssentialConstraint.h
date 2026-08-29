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
#include "TreeCotreeGaugeBuilder.h"

/**
 * Removes the gradient null space of an H(curl) (Nedelec) variable by strongly
 * fixing the lowest-order edge degrees of freedom lying on a spanning tree of
 * the mesh graph to zero (a tree-cotree gauge). This makes curl-curl systems
 * with no (or only partial) mass regularization solvable, e.g. the magnetic
 * vector potential A in a magnetodynamic A-formulation where the surrounding
 * non-conducting region carries no sigma * dA/dt term.
 *
 * The 'boundary' parameter should list exactly the boundaries on which a
 * tangential Dirichlet ("PEC") condition is applied to the variable: those
 * edges are seeded into the spanning forest so the interior gauge stays
 * compatible with the boundary condition rather than over-constraining it.
 *
 * The 'block' parameter restricts the gauge to the given subdomains (e.g. the
 * non-conducting region, where a sigma * dA/dt term does not already fix the
 * gauge). Edges of the excluded subdomains seed the forest but are not gauged.
 * An empty 'block' gauges the whole mesh.
 *
 * The spanning forest is grown from the mesh 1-skeleton gathered onto every
 * rank with edges keyed on their endpoint coordinates, so the gauge is
 * independent of the MPI partitioning.
 */
class MFEMTreeCotreeGaugeEssentialConstraint : public MFEMEssentialConstraint,
                                               public MFEMBoundaryRestrictable,
                                               protected TreeCotreeGaugeBuilder
{
public:
  static InputParameters validParams();

  MFEMTreeCotreeGaugeEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list) override;
};

#endif
