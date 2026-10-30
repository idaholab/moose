//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexTreeCotreeGaugeEssentialConstraint.h"
#include "MFEMProblem.h"
#include "MFEMConstraintUtils.h"

registerMooseObject("MooseApp", MFEMComplexTreeCotreeGaugeEssentialConstraint);

InputParameters
MFEMComplexTreeCotreeGaugeEssentialConstraint::validParams()
{
  InputParameters params = MFEMComplexEssentialConstraint::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription(
      "Applies a tree-cotree gauge to a complex H(curl) (Nedelec) variable by "
      "strongly fixing the real and imaginary parts of the lowest-order edge degrees of freedom on "
      "a spanning tree of the mesh to zero, removing the gradient null space of a curl-curl "
      "operator. List in 'boundary' the boundaries carrying a tangential Dirichlet condition on "
      "the variable. Set 'block' to the subdomains to gauge; the complement seeds the spanning "
      "forest but is not gauged. Leaving 'block' empty gauges the whole mesh. Requires a FIRST "
      "order space.");
  params.setDocString(
      "boundary",
      "The boundaries (names or ids) on which a tangential Dirichlet condition is already "
      "applied to the variable. These boundaries seed the spanning forest so the gauge stays "
      "compatible with the condition. Defaults to no boundaries.");
  return params;
}

MFEMComplexTreeCotreeGaugeEssentialConstraint::MFEMComplexTreeCotreeGaugeEssentialConstraint(
    const InputParameters & parameters)
  : MFEMComplexEssentialConstraint(parameters),
    MFEMBoundaryRestrictable(
        parameters, getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("variable")))
{
}

void
MFEMComplexTreeCotreeGaugeEssentialConstraint::ApplyConstraint(
    mfem::ParComplexGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list)
{
  const mfem::Array<int> & tree_tdofs =
      _gauge.trueDofs(*this,
                      *gridfunc.ParFESpace(),
                      isBoundaryRestricted() ? &getBoundaryMarkers() : nullptr,
                      getSubdomainMarkers());

  // The gauge fixes the tree dofs to zero in both components so the lifted
  // right-hand side stays consistent.
  Moose::MFEM::zeroTrueDofs(gridfunc.real(), tree_tdofs);
  Moose::MFEM::zeroTrueDofs(gridfunc.imag(), tree_tdofs);
  gridfunc.Sync();

  ess_tdof_list.Append(tree_tdofs);
}

#endif
