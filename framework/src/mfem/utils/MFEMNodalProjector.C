//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNodalProjector.h"
#include "MFEMVectorUtils.h"
#include "MooseError.h"

void
MFEMNodalProjector::extractNodePositions(const mfem::ParFiniteElementSpace & fespace,
                                         mfem::Vector & node_positions,
                                         mfem::Ordering::Type & node_ordering)
{
  const int nelem = fespace.GetParMesh()->GetNE();
  const int dim = fespace.GetParMesh()->Dimension();

  // Find total number of local nodes/interpolation points
  int nnodes = 0;
  for (const auto i : make_range(nelem))
    nnodes += fespace.GetFE(i)->GetNodes().GetNPoints();

  node_positions.SetSize(nnodes * dim);

  int nodal_offset = 0;
  for (const auto i : make_range(nelem))
  {
    const mfem::IntegrationRule ir = fespace.GetFE(i)->GetNodes();
    const int nqpt = ir.GetNPoints();

    mfem::DenseMatrix pos;
    fespace.GetElementTransformation(i)->Transform(ir, pos);
    mfem::Vector row;

    for (const auto d : make_range(dim))
    {
      row.SetDataAndSize(node_positions.GetData() + nodal_offset + d * nnodes, nqpt);
      pos.GetRow(d, row);
    }

    nodal_offset += nqpt;
  }
  node_ordering = mfem::Ordering::Type::byNODES;
}

void
MFEMNodalProjector::projectNodalValues(const mfem::Vector & nodal_vals,
                                       const mfem::Ordering::Type & nodal_val_ordering,
                                       mfem::ParGridFunction & gridfunction)
{
  mfem::ParFiniteElementSpace & fespace = *gridfunction.ParFESpace();
  const int nelem = fespace.GetParMesh()->GetNE();
  const int gf_ncomp = gridfunction.VectorDim();

  fespace.GetParMesh()->EnsureNodes();

  // Check FESpaces can be transferred
  const auto map = fespace.FEColl()->GetMapType(fespace.GetParMesh()->Dimension());
  const bool H1L2 = map == mfem::FiniteElement::VALUE || map == mfem::FiniteElement::INTEGRAL;
  const bool RTND = map == mfem::FiniteElement::H_DIV || map == mfem::FiniteElement::H_CURL;
  if (!H1L2 && !RTND)
    mooseError("FESpace type not supported yet in transfers.");

  // Project the interpolated values to the target FiniteElementSpace.
  mfem::Ordering::Type dof_ordering = H1L2 ? mfem::Ordering::byNODES : mfem::Ordering::byVDIM;
  mfem::Array<int> vdofs;
  mfem::Vector vals;
  int nodal_offset = 0;
  for (const auto el : make_range(nelem))
  {
    const int nqpt = fespace.GetFE(el)->GetNodes().GetNPoints();
    mfem::Vector dof_vals(nqpt * gf_ncomp);
    fespace.GetElementVDofs(el, vdofs); // Returned vdofs always indexed with ordering byNODES
    for (const auto qp : make_range(nqpt))
      for (const auto d : make_range(gf_ncomp))
        dof_vals(Moose::MFEM::MFEMIndex(d, qp, gf_ncomp, nqpt, dof_ordering)) = nodal_vals(
            Moose::MFEM::MFEMIndex(d, qp, gf_ncomp, nqpt, nodal_val_ordering) + nodal_offset);

    if (H1L2)
      gridfunction.SetSubVector(vdofs, dof_vals);
    else if (RTND)
    {
      vals.SetSize(vdofs.Size());
      fespace.GetFE(el)->ProjectFromNodes(dof_vals, *fespace.GetElementTransformation(el), vals);
      gridfunction.SetSubVector(vdofs, vals);
    }

    nodal_offset += nqpt * gf_ncomp;
  }
  gridfunction.SetTrueVector();
}

#endif
