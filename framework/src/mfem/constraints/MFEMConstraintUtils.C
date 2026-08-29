//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMConstraintUtils.h"
#include "libmesh/int_range.h"

#include <vector>

namespace Moose::MFEM
{

void
subdomainTrueDofs(mfem::ParFiniteElementSpace & pfes,
                  const mfem::Array<int> & subdomain_attrs,
                  mfem::Array<int> & tdofs)
{
  tdofs.DeleteAll();
  if (subdomain_attrs.Size() == 0)
    return;

  mfem::ParMesh & pmesh = *pfes.GetParMesh();
  const int max_attr = pmesh.attributes.Size() ? pmesh.attributes.Max() : 0;
  std::vector<char> wanted(max_attr + 1, 0);
  for (const auto a : subdomain_attrs)
    if (a >= 1 && a <= max_attr)
      wanted[a] = 1;

  // Local L-dof marker: 1 if the dof touches a local element carrying one of the
  // requested attributes. All requested attributes are marked in a single pass so
  // multi-subdomain restrictions are not clobbered.
  mfem::Array<int> dof_marker(pfes.GetVSize());
  dof_marker = 0;
  mfem::Array<int> vdofs;
  for (const auto e : make_range(pmesh.GetNE()))
    if (wanted[pmesh.GetAttribute(e)])
    {
      pfes.GetElementVDofs(e, vdofs);
      for (const auto d : vdofs)
        dof_marker[d < 0 ? -1 - d : d] = 1; // undo ND/RT sign encoding
    }

  // Make the marker consistent across processors sharing a dof (boolean-OR), then
  // convert the L-dof marker to a T-dof list exactly as GetEssentialTrueDofs does.
  pfes.Synchronize(dof_marker);
  mfem::Array<int> tdof_marker;
  pfes.GetRestrictionMatrix()->BooleanMult(dof_marker, tdof_marker);
  mfem::FiniteElementSpace::MarkerToList(tdof_marker, tdofs);
}

void
projectScalarCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                                     mfem::Coefficient & coef,
                                     const mfem::Array<int> & subdomain_attrs)
{
  if (subdomain_attrs.Size() == 0)
    return;

  mfem::FiniteElementSpace & fes = *gf.FESpace();
  const int max_attr = fes.GetMesh()->attributes.Size() ? fes.GetMesh()->attributes.Max() : 0;
  std::vector<char> wanted(max_attr + 1, 0);
  for (const auto a : subdomain_attrs)
    if (a >= 1 && a <= max_attr)
      wanted[a] = 1;

  mfem::Array<int> vdofs;
  mfem::Vector vals;
  mfem::DofTransformation doftrans;
  for (const auto e : make_range(fes.GetNE()))
  {
    if (!wanted[fes.GetAttribute(e)])
      continue;
    fes.GetElementVDofs(e, vdofs, doftrans);
    vals.SetSize(vdofs.Size());
    fes.GetFE(e)->Project(coef, *fes.GetElementTransformation(e), vals);
    doftrans.TransformPrimal(vals);
    gf.SetSubVector(vdofs, vals);
  }
}

void
projectVectorCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                                     mfem::VectorCoefficient & coef,
                                     const mfem::Array<int> & subdomain_attrs)
{
  if (subdomain_attrs.Size() == 0)
    return;

  mfem::FiniteElementSpace & fes = *gf.FESpace();
  const int max_attr = fes.GetMesh()->attributes.Size() ? fes.GetMesh()->attributes.Max() : 0;
  std::vector<char> wanted(max_attr + 1, 0);
  for (const auto a : subdomain_attrs)
    if (a >= 1 && a <= max_attr)
      wanted[a] = 1;

  mfem::Array<int> vdofs;
  mfem::Vector vals;
  mfem::DofTransformation doftrans;
  for (const auto e : make_range(fes.GetNE()))
  {
    if (!wanted[fes.GetAttribute(e)])
      continue;
    fes.GetElementVDofs(e, vdofs, doftrans);
    vals.SetSize(vdofs.Size());
    // The VectorCoefficient overload of Project handles vector H1 (one scalar
    // basis per component) and H(curl)/H(div) (tangential/normal edge/face
    // moments) alike; the DofTransformation fixes ND/RT dof orientation.
    fes.GetFE(e)->Project(coef, *fes.GetElementTransformation(e), vals);
    doftrans.TransformPrimal(vals);
    gf.SetSubVector(vdofs, vals);
  }
}
}

#endif
