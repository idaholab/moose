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
#include "MooseError.h"
#include "libmesh/int_range.h"

namespace
{

/**
 * Accumulate the local projection of @p coef over every element carrying a marked
 * attribute into @p values, counting the contributions made to each dof in
 * @p counter. Both are sized and zeroed here.
 */
template <typename CoefficientType>
void
accumulateSubdomainProjection(mfem::ParGridFunction & gf,
                              CoefficientType & coef,
                              const mfem::Array<int> & subdomain_markers,
                              mfem::Vector & values,
                              mfem::Array<int> & counter)
{
  mfem::FiniteElementSpace & fes = *gf.FESpace();
  values.SetSize(fes.GetVSize());
  values = 0.0;
  counter.SetSize(fes.GetVSize());
  counter = 0;

  mfem::Array<int> vdofs;
  mfem::Vector vals;
  mfem::DofTransformation doftrans;
  for (const auto e : libMesh::make_range(fes.GetNE()))
  {
    if (!subdomain_markers[fes.GetAttribute(e) - 1])
      continue;
    fes.GetElementVDofs(e, vdofs, doftrans);
    vals.SetSize(vdofs.Size());
    // The Project overload resolved here handles vector H1 (one scalar basis per
    // component) and H(curl)/H(div) (tangential/normal edge/face moments) alike;
    // the DofTransformation fixes ND/RT dof orientation.
    fes.GetFE(e)->Project(coef, *fes.GetElementTransformation(e), vals);
    doftrans.TransformPrimal(vals);
    // AddElementVector undoes the ND/RT sign encoding carried in vdofs.
    values.AddElementVector(vdofs, vals);
    for (const auto vdof : vdofs)
      counter[vdof < 0 ? -1 - vdof : vdof]++;
  }
}

/**
 * Average the accumulated projection over every rank that contributed to a dof and
 * write the result into the dofs of @p gf that received a contribution.
 */
void
distributeSubdomainProjection(mfem::ParGridFunction & gf,
                              mfem::Vector & values,
                              mfem::Array<int> & counter)
{
  // Sum the contributions and their multiplicity across the ranks sharing a dof,
  // then broadcast both back so every rank forms the same average. Without this a
  // dof on the subdomain boundary owned by a rank that holds no element of the
  // subdomain would keep its unprojected value, and that is the value
  // ParBilinearForm::FormLinearSystem eliminates against. This mirrors the idiom
  // used by mfem::ParGridFunction::ProjectBdrCoefficientTangent.
  mfem::GroupCommunicator & gcomm = gf.ParFESpace()->GroupComm();
  gcomm.Reduce<int>(counter.HostReadWrite(), mfem::GroupCommunicator::Sum);
  gcomm.Reduce<mfem::real_t>(values.HostReadWrite(), mfem::GroupCommunicator::Sum);

  // Ensure host data is up-to-date before writing to it.
  gf.HostReadWrite();

  for (const auto i : libMesh::make_range(gf.Size()))
    if (counter[i])
      gf(i) = values(i) / counter[i];

  gcomm.Bcast<mfem::real_t>(gf.HostReadWrite());
}

/// Shared body of the scalar and vector subdomain projections.
template <typename CoefficientType>
void
projectOnSubdomains(mfem::ParGridFunction & gf,
                    CoefficientType & coef,
                    const mfem::Array<int> & subdomain_markers)
{
  mooseAssert(gf.FESpace()->GetVectorDim() == coef.GetVDim(),
              "The coefficient must have one component per space component");

  mfem::Vector values;
  mfem::Array<int> counter;
  accumulateSubdomainProjection(gf, coef, subdomain_markers, values, counter);
  distributeSubdomainProjection(gf, values, counter);
}
}

namespace Moose::MFEM
{

void
subdomainTrueDofs(mfem::ParFiniteElementSpace & pfes,
                  const mfem::Array<int> & subdomain_markers,
                  mfem::Array<int> & tdofs)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();

  // Local L-dof marker: 1 if the dof touches a local element carrying one of the
  // requested attributes. All requested attributes are marked in a single pass so
  // multi-subdomain restrictions are not clobbered.
  mfem::Array<int> dof_marker(pfes.GetVSize());
  dof_marker = 0;
  mfem::Array<int> vdofs;
  for (const auto e : libMesh::make_range(pmesh.GetNE()))
    if (subdomain_markers[pmesh.GetAttribute(e) - 1])
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
projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                               mfem::Coefficient & coef,
                               const mfem::Array<int> & subdomain_markers)
{
  projectOnSubdomains(gf, coef, subdomain_markers);
}

void
projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                               mfem::VectorCoefficient & coef,
                               const mfem::Array<int> & subdomain_markers)
{
  projectOnSubdomains(gf, coef, subdomain_markers);
}

void
zeroTrueDofs(mfem::ParGridFunction & gf, const mfem::Array<int> & tdofs)
{
  // Go through the true vector so the local vector is left consistent across the
  // ranks sharing a dof.
  gf.SetTrueVector();
  gf.GetTrueVector().SetSubVector(tdofs, 0.);
  gf.SetFromTrueVector();
}
}

#endif
