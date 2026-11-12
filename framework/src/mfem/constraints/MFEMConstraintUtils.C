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
      counter[mfem::FiniteElementSpace::DecodeDof(vdof)]++;
  }
}

/**
 * Average the accumulated projection over every rank that contributed to a dof and
 * write the result into the dofs of @p gf marked in @p dof_marker that received a
 * contribution.
 */
void
distributeSubdomainProjection(mfem::ParGridFunction & gf,
                              mfem::Vector & values,
                              mfem::Array<int> & counter,
                              const mfem::Array<int> & dof_marker)
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

  // Unmarked dofs on the subdomain boundary still received a contribution, but they
  // are not constrained and must keep their values, which may come from an essential
  // boundary condition.
  for (const auto i : libMesh::make_range(gf.Size()))
    if (counter[i] && dof_marker[i])
      gf(i) = values(i) / counter[i];

  gcomm.Bcast<mfem::real_t>(gf.HostReadWrite());
}

/// Shared body of the scalar and vector subdomain projections.
template <typename CoefficientType>
void
projectOnSubdomains(mfem::ParGridFunction & gf,
                    CoefficientType & coef,
                    const mfem::Array<int> & subdomain_markers,
                    const mfem::Array<int> & dof_marker)
{
  mooseAssert(gf.FESpace()->GetVectorDim() == coef.GetVDim(),
              "The coefficient must have one component per space component");

  mfem::Vector values;
  mfem::Array<int> counter;
  accumulateSubdomainProjection(gf, coef, subdomain_markers, values, counter);
  distributeSubdomainProjection(gf, values, counter, dof_marker);
}
}

namespace Moose::MFEM
{

void
subdomainDofs(mfem::ParFiniteElementSpace & pfes,
              const mfem::Array<int> & subdomain_markers,
              const bool include_boundary,
              mfem::Array<int> & dof_marker,
              mfem::Array<int> & tdofs)
{
  mfem::ParMesh & pmesh = *pfes.GetParMesh();

  // Local L-dof markers: dof_marker is 1 if the dof touches a local element carrying
  // one of the requested attributes, outside_marker if it touches one that does not.
  // All requested attributes are marked in a single pass so multi-subdomain
  // restrictions are not clobbered.
  dof_marker.SetSize(pfes.GetVSize());
  dof_marker = 0;
  mfem::Array<int> outside_marker(pfes.GetVSize());
  outside_marker = 0;
  mfem::Array<int> vdofs;
  for (const auto e : libMesh::make_range(pmesh.GetNE()))
  {
    pfes.GetElementVDofs(e, vdofs);
    auto & marker = subdomain_markers[pmesh.GetAttribute(e) - 1] ? dof_marker : outside_marker;
    for (const auto d : vdofs)
      marker[mfem::FiniteElementSpace::DecodeDof(d)] = 1; // undo ND/RT sign encoding
  }

  // Make the marker consistent across processors sharing a dof (boolean-OR).
  pfes.Synchronize(dof_marker);

  if (!include_boundary)
  {
    // The subdomain boundary consists of the dofs the subdomains share with other
    // elements, marked above, and those on the exterior boundary of the mesh.
    // Periodic faces and nonconforming master faces are not exterior, and neither
    // are faces shared between ranks. GetFaceInformation only tells those apart once
    // ParMesh::ExchangeFaceNbrData has run, so they are skipped explicitly.
    std::vector<bool> shared_face(pmesh.GetNumFaces(), false);
    for (const auto i : libMesh::make_range(pmesh.GetNSharedFaces()))
      shared_face[pmesh.GetSharedFace(i)] = true;
    for (const auto f : libMesh::make_range(pmesh.GetNumFaces()))
      if (!shared_face[f] && pmesh.GetFaceInformation(f).IsBoundary())
      {
        pfes.GetFaceVDofs(f, vdofs);
        for (const auto d : vdofs)
          outside_marker[mfem::FiniteElementSpace::DecodeDof(d)] = 1;
      }
    // Synchronizing makes an interface coinciding with a processor boundary visible
    // on the rank holding only the subdomain side of it.
    pfes.Synchronize(outside_marker);
    for (const auto i : libMesh::make_range(dof_marker.Size()))
      dof_marker[i] = dof_marker[i] && !outside_marker[i];
  }

  // Convert the L-dof marker to a T-dof list exactly as GetEssentialTrueDofs does.
  mfem::Array<int> tdof_marker;
  pfes.GetRestrictionMatrix()->BooleanMult(dof_marker, tdof_marker);
  mfem::FiniteElementSpace::MarkerToList(tdof_marker, tdofs);
}

void
projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                               mfem::Coefficient & coef,
                               const mfem::Array<int> & subdomain_markers,
                               const mfem::Array<int> & dof_marker)
{
  projectOnSubdomains(gf, coef, subdomain_markers, dof_marker);
}

void
projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                               mfem::VectorCoefficient & coef,
                               const mfem::Array<int> & subdomain_markers,
                               const mfem::Array<int> & dof_marker)
{
  projectOnSubdomains(gf, coef, subdomain_markers, dof_marker);
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
