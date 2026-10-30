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
#include "MFEMConstraintUtils.h"

registerMooseObject("MooseApp", MFEMComplexTreeCotreeGaugeEssentialConstraint);

InputParameters
MFEMComplexTreeCotreeGaugeEssentialConstraint::validParams()
{
  InputParameters params = MFEMComplexEssentialConstraint::validParams();
  params.addClassDescription(
      "Applies a tree-cotree gauge to a complex H(curl) (Nedelec) variable by "
      "strongly fixing the real and imaginary parts of the edge degrees of freedom on a spanning "
      "tree of the mesh to zero, and in a higher order space those of a selection of the degrees "
      "of freedom of every other edge, face and element, removing the gradient null space of a "
      "curl-curl operator. Edges carrying an essential condition on the variable "
      "seed the spanning forest. Set 'block' to the subdomains to gauge; the complement seeds the "
      "spanning forest but is not gauged. Leaving 'block' empty gauges the whole mesh. Above "
      "SECOND order, the mesh must have no triangular faces.");
  return params;
}

MFEMComplexTreeCotreeGaugeEssentialConstraint::MFEMComplexTreeCotreeGaugeEssentialConstraint(
    const InputParameters & parameters)
  : MFEMComplexEssentialConstraint(parameters)
{
}

void
MFEMComplexTreeCotreeGaugeEssentialConstraint::ApplyConstraint(
    mfem::ParComplexGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list)
{
  const mfem::Array<int> & tree_tdofs =
      _gauge.trueDofs(*this, *gridfunc.ParFESpace(), ess_tdof_list, getSubdomainMarkers());

  // The gauge fixes the tree dofs to zero in both components so the lifted
  // right-hand side stays consistent. The real and imaginary views are brought up
  // to date with the complex vector before being written, and hand their state
  // back afterwards.
  gridfunc.Sync();
  Moose::MFEM::zeroTrueDofs(gridfunc.real(), tree_tdofs);
  Moose::MFEM::zeroTrueDofs(gridfunc.imag(), tree_tdofs);
  gridfunc.SyncAlias();

  ess_tdof_list.Append(tree_tdofs);
}

#endif
