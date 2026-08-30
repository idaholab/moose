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
#include "MFEMProblem.h"
#include "MFEMConstraintUtils.h"

registerMooseObject("MooseApp", MFEMTreeCotreeGaugeEssentialConstraint);

InputParameters
MFEMTreeCotreeGaugeEssentialConstraint::validParams()
{
  InputParameters params = MFEMEssentialConstraint::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription(
      "Applies a tree-cotree gauge to an H(curl) (Nedelec) variable by strongly fixing the "
      "lowest-order edge degrees of freedom on a spanning tree of the mesh to zero, removing the "
      "gradient null space of a curl-curl operator. List in 'boundary' the boundaries carrying a "
      "tangential Dirichlet condition on the variable so the gauge stays compatible with it. Set "
      "'block' to the subdomains to gauge; the complement is treated as already gauged and only "
      "seeds the spanning forest. Leaving 'block' empty gauges the whole mesh. Requires a FIRST "
      "order space.");
  return params;
}

MFEMTreeCotreeGaugeEssentialConstraint::MFEMTreeCotreeGaugeEssentialConstraint(
    const InputParameters & parameters)
  : MFEMEssentialConstraint(parameters),
    MFEMBoundaryRestrictable(
        parameters, getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("variable")))
{
}

void
MFEMTreeCotreeGaugeEssentialConstraint::ApplyConstraint(mfem::ParGridFunction & gridfunc,
                                                        mfem::Array<int> & ess_tdof_list)
{
  const mfem::Array<int> & tree_tdofs =
      _gauge.trueDofs(*this,
                      *gridfunc.ParFESpace(),
                      isBoundaryRestricted() ? &getBoundaryMarkers() : nullptr,
                      getSubdomainAttributes());

  // The gauge fixes the tree dofs to zero: zero those entries of the solution
  // gridfunction so the lifted right-hand side stays consistent.
  Moose::MFEM::zeroTrueDofs(gridfunc, tree_tdofs);

  ess_tdof_list.Append(tree_tdofs);
}

#endif
