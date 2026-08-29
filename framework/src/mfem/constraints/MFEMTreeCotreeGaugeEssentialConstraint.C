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
      "'block' to the subdomains to gauge (e.g. the non-conducting region of an A-formulation "
      "eddy-current problem); the complement is treated as already gauged and only seeds the "
      "spanning forest. Leaving 'block' empty gauges the whole mesh.");
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
  mfem::ParFiniteElementSpace & pfes = *gridfunc.ParFESpace();
  if (pfes.GetMaxElementOrder() > 1)
    mooseError("The tree-cotree gauge fixes the lowest-order edge degrees of freedom only, so it "
               "requires a FIRST order H(curl) space; the space of variable '",
               getTrialVariableName(),
               "' is order ",
               pfes.GetMaxElementOrder(),
               ". The gradient modes carried by the higher-order degrees of freedom would be left "
               "in place and the system would stay singular.");

  const mfem::Array<int> & tree_tdofs = _gauge.trueDofs(
      pfes, isBoundaryRestricted() ? &getBoundaryMarkers() : nullptr, getSubdomainAttributes());

  // The gauge fixes the tree dofs to zero: zero those entries of the solution
  // gridfunction so the lifted right-hand side stays consistent.
  gridfunc.SetTrueVector();
  mfem::Vector & true_dofs = gridfunc.GetTrueVector();
  for (const auto tdof : tree_tdofs)
    true_dofs(tdof) = 0.0;
  gridfunc.SetFromTrueVector();

  ess_tdof_list.Append(tree_tdofs);
}

#endif
