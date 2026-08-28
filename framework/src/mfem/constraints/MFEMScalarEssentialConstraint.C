//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarEssentialConstraint.h"

registerMooseObject("MooseApp", MFEMScalarEssentialConstraint);

InputParameters
MFEMScalarEssentialConstraint::validParams()
{
  InputParameters params = MFEMEssentialConstraint::validParams();
  params.addClassDescription("Strongly constrains a scalar variable in the specified domain.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "0.", "The coefficient setting the values on the essential boundary");
  return params;
}

MFEMScalarEssentialConstraint::MFEMScalarEssentialConstraint(const InputParameters & parameters)
  : MFEMEssentialConstraint(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

void
MFEMScalarEssentialConstraint::ApplyConstraint(mfem::ParGridFunction & gridfunc,
                                               mfem::Array<int> & ess_tdof_list)
{
  mfem::Array<int> vdofs;
  mfem::Vector vals;
  mfem::DofTransformation doftrans;
  mfem::FiniteElementSpace * fes = gridfunc.FESpace();
  const mfem::Array<int> & domain_attrs = getSubdomainAttributes();

  for (const auto attribute : domain_attrs)
  {
    GetSubdomainTrueDofs(gridfunc, attribute, ess_tdof_list);
    for (int i = 0; i < fes->GetNE(); i++)
    {
      if (fes->GetAttribute(i) != attribute)
        continue;

      fes->GetElementVDofs(i, vdofs, doftrans);
      vals.SetSize(vdofs.Size());
      fes->GetFE(i)->Project(_coef, *fes->GetElementTransformation(i), vals);
      doftrans.TransformPrimal(vals);
      gridfunc.SetSubVector(vdofs, vals);
    }
  }
}
// Returns the list of true-dof indices (in the numbering of gf's
// ParFiniteElementSpace) that belong to elements carrying mesh
// attribute `attr`. Correct in parallel: dofs shared across an
// inter-processor boundary that coincides with the subdomain boundary
// are picked up even on ranks whose own elements don't have `attr`.
void
MFEMScalarEssentialConstraint::GetSubdomainTrueDofs(const mfem::ParGridFunction & gf,
                                                    int attr,
                                                    mfem::Array<int> & ess_tdofs)
{
  mfem::ParFiniteElementSpace * fespace = gf.ParFESpace();
  mfem::ParMesh * pmesh = fespace->GetParMesh();

  // 1. Local marker over L-dofs (vdofs): 1 if the dof touches at
  //    least one *local* element with the requested attribute.
  mfem::Array<int> dof_marker(fespace->GetVSize());
  dof_marker = 0;

  mfem::Array<int> vdofs;
  for (int e = 0; e < pmesh->GetNE(); e++)
  {
    if (pmesh->GetAttribute(e) == attr)
    {
      fespace->GetElementVDofs(e, vdofs);
      for (int i = 0; i < vdofs.Size(); i++)
      {
        // Undo the sign encoding used for vector-valued (ND/RT) dofs.
        int j = (vdofs[i] >= 0) ? vdofs[i] : -1 - vdofs[i];
        dof_marker[j] = 1;
      }
    }
  }

  // 2. Synchronize the marker across processors sharing a dof: this
  //    does a boolean-OR reduce + broadcast over the dof groups, so a
  //    shared dof ends up marked on every rank that touches it, even
  //    if that rank's own elements don't carry `attr`.
  fespace->Synchronize(dof_marker);

  // 3. Convert the (now-consistent) L-dof marker to a T-dof marker,
  //    exactly as GetEssentialTrueDofs does, then list the indices.
  mfem::Array<int> true_dof_marker;
  fespace->GetRestrictionMatrix()->BooleanMult(dof_marker, true_dof_marker);

  mfem::FiniteElementSpace::MarkerToList(true_dof_marker, ess_tdofs);
}

#endif
