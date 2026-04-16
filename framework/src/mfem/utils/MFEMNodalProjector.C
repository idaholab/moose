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
#include "MooseError.h"

void
MFEMNodalProjector::extractNodePositions(const mfem::ParFiniteElementSpace & fespace,
                                         mfem::Vector & node_positions,
                                         mfem::Ordering::Type & node_ordering)
{
  const int nelem = fespace.GetParMesh()->GetNE();
  const int nqpt = fespace.GetTypicalFE()->GetNodes().GetNPoints();
  const int dim = fespace.GetParMesh()->Dimension();

  node_positions.SetSize(nqpt * nelem * dim);
  for (int i = 0; i < nelem; i++)
  {
    const mfem::FiniteElement * fe = fespace.GetFE(i);
    const mfem::IntegrationRule ir = fe->GetNodes();
    mfem::ElementTransformation * et = fespace.GetElementTransformation(i);

    mfem::DenseMatrix pos;
    et->Transform(ir, pos);
    mfem::Vector rowx(node_positions.GetData() + i * nqpt, nqpt),
        rowy(node_positions.GetData() + i * nqpt + nelem * nqpt, nqpt), rowz;
    if (dim == 3)
      rowz.SetDataAndSize(node_positions.GetData() + i * nqpt + 2 * nelem * nqpt, nqpt);

    pos.GetRow(0, rowx);
    pos.GetRow(1, rowy);
    if (dim == 3)
      pos.GetRow(2, rowz);
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
  const int nqpt = fespace.GetTypicalFE()->GetNodes().GetNPoints();
  const int gf_ncomp = gridfunction.VectorDim();

  fespace.GetParMesh()->EnsureNodes();

  // Check FESpaces can be transferred
  int fieldtype;
  {
    const mfem::FiniteElementCollection * fec_in = fespace.FEColl();
    const mfem::H1_FECollection * fec_h1 = dynamic_cast<const mfem::H1_FECollection *>(fec_in);
    const mfem::L2_FECollection * fec_l2 = dynamic_cast<const mfem::L2_FECollection *>(fec_in);
    const mfem::RT_FECollection * fec_rt = dynamic_cast<const mfem::RT_FECollection *>(fec_in);
    const mfem::ND_FECollection * fec_nd = dynamic_cast<const mfem::ND_FECollection *>(fec_in);
    if (fec_h1)
      fieldtype = 0;
    else if (fec_l2)
      fieldtype = 1;
    else if (fec_rt)
      fieldtype = 2;
    else if (fec_nd)
      fieldtype = 3;
    else
      mooseError("FESpace type not supported yet in transfers.");
  }

  // Project the interpolated values to the target FiniteElementSpace.
  mfem::Array<int> vdofs;
  mfem::Vector vals;
  mfem::Vector elem_dof_vals(nqpt * gf_ncomp);
  if (fieldtype <= 1) // H1 or L2
  {
    for (int el = 0; el < nelem; el++)
    {
      fespace.GetElementVDofs(el, vdofs); // Returned vdofs always indexed with ordering byNODES
      for (int qp = 0; qp < nqpt; qp++)
        for (int d = 0; d < gf_ncomp; d++)
          switch (nodal_val_ordering)
          {
            case mfem::Ordering::Type::byNODES: // nqpt x VDIM x nelem
              elem_dof_vals(qp + d * nqpt) = nodal_vals(qp + nqpt * (d + gf_ncomp * el));
              break;
            case mfem::Ordering::Type::byVDIM: // VDIM x nqpt x nelem
              elem_dof_vals(qp + d * nqpt) = nodal_vals(d + gf_ncomp * (qp + nqpt * el));
              break;
          }
      gridfunction.SetSubVector(vdofs, elem_dof_vals);
    }
  }
  else // H(div) or H(curl)
  {
    for (int el = 0; el < nelem; el++)
    {
      fespace.GetElementVDofs(el, vdofs);
      for (int qp = 0; qp < nqpt; qp++)
        for (int d = 0; d < gf_ncomp; d++)
          switch (nodal_val_ordering) // elem_dof_vals ordering chosen for ProjectFromNodes
          {
            case mfem::Ordering::Type::byNODES: // nqpt x VDIM x nelem
              elem_dof_vals(d + gf_ncomp * qp) = nodal_vals(qp + nqpt * (d + gf_ncomp * el));
              break;
            case mfem::Ordering::Type::byVDIM: // VDIM x nqpt x nelem
              elem_dof_vals(d + gf_ncomp * qp) = nodal_vals(d + gf_ncomp * (qp + nqpt * el));
              break;
          }
      vals.SetSize(vdofs.Size());
      fespace.GetFE(el)->ProjectFromNodes(
          elem_dof_vals, *fespace.GetElementTransformation(el), vals);
      gridfunction.SetSubVector(vdofs, vals);
    }
  }
  gridfunction.SetTrueVector();
}

#endif
