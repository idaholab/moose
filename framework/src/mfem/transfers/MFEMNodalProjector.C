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
  const int NE = fespace.GetParMesh()->GetNE();
  const int nsp = fespace.GetTypicalFE()->GetNodes().GetNPoints();
  const int dim = fespace.GetParMesh()->Dimension();

  node_positions.SetSize(nsp * NE * dim);
  for (int i = 0; i < NE; i++)
  {
    const mfem::FiniteElement * fe = fespace.GetFE(i);
    const mfem::IntegrationRule ir = fe->GetNodes();
    mfem::ElementTransformation * et = fespace.GetElementTransformation(i);

    mfem::DenseMatrix pos;
    et->Transform(ir, pos);
    mfem::Vector rowx(node_positions.GetData() + i * nsp, nsp),
        rowy(node_positions.GetData() + i * nsp + NE * nsp, nsp), rowz;
    if (dim == 3)
      rowz.SetDataAndSize(node_positions.GetData() + i * nsp + 2 * NE * nsp, nsp);

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
  const int NE = fespace.GetParMesh()->GetNE();
  const int nsp = fespace.GetTypicalFE()->GetNodes().GetNPoints();
  const int dim = fespace.GetParMesh()->Dimension();
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
    {
      fieldtype = 0;
    }
    else if (fec_l2)
    {
      fieldtype = 1;
    }
    else if (fec_rt)
    {
      fieldtype = 2;
    }
    else if (fec_nd)
    {
      fieldtype = 3;
    }
    else
    {
      mooseError("FESpace type not supported yet in transfers.");
    }
  }

  // Project the interpolated values to the target FiniteElementSpace.
  if (fieldtype <= 1) // H1 or L2
  {
    mfem::Array<int> vdofs;
    mfem::Vector vals;
    mfem::Vector elem_dof_vals(nsp * gf_ncomp);
    for (int i = 0; i < NE; i++)
    {
      fespace.GetElementVDofs(i, vdofs);
      vals.SetSize(vdofs.Size());
      for (int j = 0; j < nsp; j++)
      {
        for (int d = 0; d < gf_ncomp; d++)
        {
          int idx;
          if (gf_ncomp > 1)
            idx = nodal_val_ordering == mfem::Ordering::Type::byNODES ? d * nsp * NE + i * nsp + j
                                                                      : i * nsp * dim + d + j * dim;
          else
            idx = d * nsp * NE + i * nsp + j;
          elem_dof_vals(j + d * nsp) = nodal_vals(idx);
        }
      }
      gridfunction.SetSubVector(vdofs, elem_dof_vals);
    }
  }
  else // H(div) or H(curl)
  {
    mfem::Array<int> vdofs;
    mfem::Vector vals;
    mfem::Vector elem_dof_vals(nsp * gf_ncomp);

    for (int i = 0; i < NE; i++)
    {
      fespace.GetElementVDofs(i, vdofs);
      vals.SetSize(vdofs.Size());
      for (int j = 0; j < nsp; j++)
      {
        for (int d = 0; d < gf_ncomp; d++)
        {
          int idx = nodal_val_ordering == mfem::Ordering::Type::byNODES
                        ? d * nsp * NE + i * nsp + j
                        : i * nsp * dim + d + j * dim;
          elem_dof_vals(j * gf_ncomp + d) = nodal_vals(idx);
        }
      }
      fespace.GetFE(i)->ProjectFromNodes(elem_dof_vals, *fespace.GetElementTransformation(i), vals);
      gridfunction.SetSubVector(vdofs, vals);
    }
  }
  gridfunction.SetFromTrueVector();
}

#endif
