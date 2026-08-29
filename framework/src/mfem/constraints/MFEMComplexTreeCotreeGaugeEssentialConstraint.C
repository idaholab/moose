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

registerMooseObject("MooseApp", MFEMComplexTreeCotreeGaugeEssentialConstraint);

InputParameters
MFEMComplexTreeCotreeGaugeEssentialConstraint::validParams()
{
  InputParameters params = MFEMComplexEssentialConstraint::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription(
      "Applies a tree-cotree gauge to a complex (time-harmonic) H(curl) (Nedelec) variable by "
      "strongly fixing the real and imaginary parts of the lowest-order edge degrees of freedom on "
      "a spanning tree of the mesh to zero, removing the gradient null space of a curl-curl "
      "operator. List in 'boundary' the boundaries carrying a tangential Dirichlet condition on "
      "the "
      "variable. Set 'block' to the subdomains to gauge (e.g. the non-conducting region of a "
      "time-harmonic eddy-current problem); the complement seeds the spanning forest but is not "
      "gauged. Leaving 'block' empty gauges the whole mesh.");
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
  mfem::ParFiniteElementSpace & pfes = *gridfunc.ParFESpace();

  mfem::Array<int> tree_tdofs = gaugeTrueDofs(
      pfes, isBoundaryRestricted() ? &getBoundaryMarkers() : nullptr, getSubdomainAttributes());

  // The gauge fixes the tree dofs to zero in both components so the lifted
  // right-hand side stays consistent.
  for (mfem::ParGridFunction * component : {&gridfunc.real(), &gridfunc.imag()})
  {
    component->SetTrueVector();
    mfem::Vector & true_dofs = component->GetTrueVector();
    for (const auto tdof : tree_tdofs)
      true_dofs(tdof) = 0.0;
    component->SetFromTrueVector();
  }
  gridfunc.Sync();

  ess_tdof_list.Append(tree_tdofs);
}

#endif
