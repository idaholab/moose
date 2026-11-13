//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMConstraint.h"
#include "MFEMProblem.h"
#include "MFEMConstraintUtils.h"

InputParameters
MFEMConstraint::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params += MFEMBlockRestrictable::validParams();

  params.addClassDescription("Base class for constraining an MFEM variable within one or more mesh "
                             "subdomains.");
  params.registerBase("Constraint");
  params.registerSystemAttributeName("Constraint");
  params.addParam<VariableName>("variable", "Variable on which to apply the constraint");
  params.addParam<bool>(
      "constrain_subdomain_boundary",
      true,
      "Whether to constrain the degrees of freedom on the boundary of the subdomains given by "
      "'block', both on interfaces with other subdomains and on the exterior boundary of the "
      "mesh. If false, only the degrees of freedom interior to the subdomains are constrained.");
  return params;
}

MFEMConstraint::MFEMConstraint(const InputParameters & parameters)
  : MFEMObject(parameters),
    MFEMBlockRestrictable(parameters,
                          getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("variable"))),
    _trial_var_name(getParam<VariableName>("variable")),
    _constrain_subdomain_boundary(getParam<bool>("constrain_subdomain_boundary"))
{
}

void
MFEMConstraint::checkDisjointSubdomains(MFEMConstraint & other)
{
  // Both marker arrays are built from the mesh of the same variable, so they have
  // the same size. An empty 'block' marks every subdomain.
  const auto & markers = getSubdomainMarkers();
  const auto & other_markers = other.getSubdomainMarkers();
  for (const auto i : make_range(markers.Size()))
    if (markers[i] && other_markers[i])
      paramError("block",
                 "shares subdomain ",
                 i + 1,
                 " with the constraint '",
                 other.name(),
                 "' on variable '",
                 getTrialVariableName(),
                 "'. Essential constraints on the same variable must be restricted to disjoint "
                 "sets of subdomains.");
}

const mfem::Array<int> &
MFEMConstraint::getSubdomainTrueDofs(mfem::ParFiniteElementSpace & pfes)
{
  updateSubdomainDofs(pfes);
  return _subdomain_tdofs;
}

const mfem::Array<int> &
MFEMConstraint::getSubdomainDofMarker(mfem::ParFiniteElementSpace & pfes)
{
  updateSubdomainDofs(pfes);
  return _subdomain_dof_marker;
}

void
MFEMConstraint::updateSubdomainDofs(mfem::ParFiniteElementSpace & pfes)
{
  // MFEM bumps the sequence number whenever the space is updated, including after
  // refinement of the mesh, so it invalidates the cache exactly when needed.
  if (_subdomain_tdofs_sequence != pfes.GetSequence())
  {
    Moose::MFEM::subdomainDofs(pfes,
                               getSubdomainMarkers(),
                               _constrain_subdomain_boundary,
                               _subdomain_dof_marker,
                               _subdomain_tdofs);
    _subdomain_tdofs_sequence = pfes.GetSequence();
  }
}

#endif
