//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTransferProjector.h"

void
MFEMTransferProjector::extractProjectionPoints(const mfem::ParFiniteElementSpace & to_pfespace, mfem::Vector & vxyz, mfem::Ordering::Type & point_ordering)
{
  const int NE = to_pfespace.GetParMesh()->GetNE();
  const int nsp = to_pfespace.GetTypicalFE()->GetNodes().GetNPoints();
  const int dim = to_pfespace.GetParMesh()->Dimension();

  vxyz.SetSize(nsp*NE*dim);
  for (int i = 0; i < NE; i++)
  {
      const mfem::FiniteElement *fe = to_pfespace.GetFE(i);
      const mfem::IntegrationRule ir = fe->GetNodes();
      mfem::ElementTransformation *et = to_pfespace.GetElementTransformation(i);

      mfem::DenseMatrix pos;
      et->Transform(ir, pos);
      mfem::Vector rowx(vxyz.GetData() + i*nsp, nsp),
            rowy(vxyz.GetData() + i*nsp + NE*nsp, nsp),
            rowz;
      if (dim == 3)
      {
        rowz.SetDataAndSize(vxyz.GetData() + i*nsp + 2*NE*nsp, nsp);
      }
      pos.GetRow(0, rowx);
      pos.GetRow(1, rowy);
      if (dim == 3) { pos.GetRow(2, rowz); }
  }
  point_ordering = mfem::Ordering::Type::byNODES;
}

void
MFEMTransferProjector::projectValues(const mfem::Vector & interp_vals, const mfem::Ordering::Type & interp_value_ordering, mfem::ParGridFunction & to_gf)
{
  mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();
  const int NE = to_pfespace.GetParMesh()->GetNE();
  const int nsp = to_pfespace.GetTypicalFE()->GetNodes().GetNPoints();
  const int dim = to_pfespace.GetParMesh()->Dimension();
  const int to_gf_ncomp = to_gf.VectorDim();

  to_pfespace.GetParMesh()->EnsureNodes();
  
  // Check FESpaces can be transferred
  int fieldtype;
  {
    const mfem::FiniteElementCollection *fec_in = to_pfespace.FEColl();
    const mfem::H1_FECollection *fec_h1 = dynamic_cast<const mfem::H1_FECollection *>(fec_in);
    const mfem::L2_FECollection *fec_l2 = dynamic_cast<const mfem::L2_FECollection *>(fec_in);
    const mfem::RT_FECollection *fec_rt = dynamic_cast<const mfem::RT_FECollection *>(fec_in);
    const mfem::ND_FECollection *fec_nd = dynamic_cast<const mfem::ND_FECollection *>(fec_in);
    if (fec_h1)      { fieldtype = 0; }
    else if (fec_l2) { fieldtype = 1; }
    else if (fec_rt) { fieldtype = 2; }
    else if (fec_nd) { fieldtype = 3; }
    else { mooseError("FESpace type not supported yet in transfers."); }
  }

  // Project the interpolated values to the target FiniteElementSpace.
  if (fieldtype <= 1) // H1 or L2
  {
    mfem::Array<int> vdofs;
    mfem::Vector vals;
    mfem::Vector elem_dof_vals(nsp*to_gf_ncomp);
    for (int i = 0; i < NE; i++)
    {
      to_pfespace.GetElementVDofs(i, vdofs);
      vals.SetSize(vdofs.Size());
      for (int j = 0; j < nsp; j++)
      {
        for (int d = 0; d < to_gf_ncomp; d++)
        {
          int idx;
          if (to_gf_ncomp>1)
            idx = interp_value_ordering == mfem::Ordering::Type::byNODES ?
                      d*nsp*NE + i*nsp + j : i*nsp*dim + d + j*dim;
          else
            idx = d*nsp*NE + i*nsp + j;
          elem_dof_vals(j + d*nsp) = interp_vals(idx);
        }
      }
      to_gf.SetSubVector(vdofs, elem_dof_vals);
    }
  }
  else // H(div) or H(curl)
  {
    mfem::Array<int> vdofs;
    mfem::Vector vals;
    mfem::Vector elem_dof_vals(nsp*to_gf_ncomp);

    for (int i = 0; i < NE; i++)
    {
      to_pfespace.GetElementVDofs(i, vdofs);
      vals.SetSize(vdofs.Size());
      for (int j = 0; j < nsp; j++)
      {
        for (int d = 0; d < to_gf_ncomp; d++)
        {
          // Arrange values byVDim
          int idx = interp_value_ordering == mfem::Ordering::Type::byNODES ?
                    d*nsp*NE + i*nsp + j : i*nsp*dim + d + j*dim;
          elem_dof_vals(j*to_gf_ncomp+d) = interp_vals(idx);
        }
      }
      to_pfespace.GetFE(i)->ProjectFromNodes(elem_dof_vals,
                                          *to_pfespace.GetElementTransformation(i),
                                          vals);
      to_gf.SetSubVector(vdofs, vals);
    }
  }
  to_gf.SetFromTrueVector();
}

#endif
