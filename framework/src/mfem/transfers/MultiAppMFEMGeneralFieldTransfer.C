//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiAppMFEMGeneralFieldTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"

registerMooseObject("MooseApp", MultiAppMFEMGeneralFieldTransfer);

InputParameters
MultiAppMFEMGeneralFieldTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "AuxVariable to store transferred value in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "Variable to transfer from");
  params.addClassDescription("Copies variable values from one MFEM application to another");
  return params;
}

MultiAppMFEMGeneralFieldTransfer::MultiAppMFEMGeneralFieldTransfer(InputParameters const & params)
  : MultiAppTransfer(params),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variable"))
{
  auto bad_problem = [this]()
  {
    mooseError(type(),
               " only works with MFEMProblem based applications. Check that all your inputs "
               "involved in this transfer are MFEMProblem based");
  };
  if (hasToMultiApp())
  {
    if (!dynamic_cast<MFEMProblem *>(&getToMultiApp()->problemBase()))
      bad_problem();
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getToMultiApp()->hasLocalApp(i) &&
          !dynamic_cast<MFEMProblem *>(&getToMultiApp()->appProblemBase(i)))
        bad_problem();
  }
  if (hasFromMultiApp())
  {
    if (!dynamic_cast<MFEMProblem *>(&getFromMultiApp()->problemBase()))
      bad_problem();
    for (const auto i : make_range(getFromMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) &&
          !dynamic_cast<MFEMProblem *>(&getFromMultiApp()->appProblemBase(i)))
        bad_problem();
  }
}

void
MultiAppMFEMGeneralFieldTransfer::extractOutgoingPoints(mfem::ParFiniteElementSpace & to_pfespace, mfem::Vector & vxyz, mfem::Ordering::Type & point_ordering)
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
MultiAppMFEMGeneralFieldTransfer::transfer(MFEMProblem & to_problem, MFEMProblem & from_problem)
{
  if (numToVar() != numFromVar())
    mooseError("Number of variables transferred must be same in both systems.");

  for (const auto v : make_range(numToVar()))
  {
    mfem::ParGridFunction & from_gf = *from_problem.getProblemData().gridfunctions.Get(getFromVarName(v));
    mfem::ParGridFunction & to_gf = *to_problem.getProblemData().gridfunctions.Get(getToVarName(v));

    mfem::ParFiniteElementSpace & from_pfespace = *from_gf.ParFESpace();
    mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();

    from_pfespace.GetParMesh()->EnsureNodes();
    to_pfespace.GetParMesh()->EnsureNodes();
    
    // Check FESpaces can be transferred
    int fieldtype;
    {
      const mfem::FiniteElementCollection *fec_in = from_pfespace.FEColl();
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

    // Generate list of points where the grid function will be evaluated
    mfem::Vector vxyz;
    mfem::Ordering::Type point_ordering;    
    extractOutgoingPoints(to_pfespace, vxyz, point_ordering);

    const int NE = to_pfespace.GetParMesh()->GetNE();
    const int nsp = to_pfespace.GetTypicalFE()->GetNodes().GetNPoints();
    const int dim = to_pfespace.GetParMesh()->Dimension();

    // Evaluate source grid function at target points
    const int nodes_cnt = vxyz.Size() / dim;
    const int to_gf_ncomp = to_gf.VectorDim();
    mfem::Vector interp_vals(nodes_cnt*to_gf_ncomp);
    _mfem_interpolator.Interpolate(*from_gf.ParFESpace()->GetParMesh(), vxyz, from_gf, interp_vals, point_ordering);
    
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
              // Arrange values byNodes
              int idx = from_pfespace.GetOrdering() == mfem::Ordering::Type::byNODES ?
                        i*nsp*dim + d + j*dim : d*nsp*NE + i*nsp + j;
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
                int idx = from_pfespace.GetOrdering() == mfem::Ordering::Type::byNODES ?
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
}

void
MultiAppMFEMGeneralFieldTransfer::execute()
{
  TIME_SECTION("MultiAppMFEMGeneralFieldTransfer::execute", 5, "Copies variables");
  if (_current_direction == TO_MULTIAPP)
  {
    for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    {
      if (getToMultiApp()->hasLocalApp(i))
      {
        transfer(static_cast<MFEMProblem &>(getToMultiApp()->appProblemBase(i)),
                 static_cast<MFEMProblem &>(getToMultiApp()->problemBase()));
      }
    }
  }
  else if (_current_direction == FROM_MULTIAPP)
  {
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        transfer(static_cast<MFEMProblem &>(getFromMultiApp()->problemBase()),
                 static_cast<MFEMProblem &>(getFromMultiApp()->appProblemBase(i)));
      }
    }
  }
  else if (_current_direction == BETWEEN_MULTIAPP)
  {
    int transfers_done = 0;
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        if (getToMultiApp()->hasLocalApp(i))
        {
          transfer(static_cast<MFEMProblem &>(getToMultiApp()->appProblemBase(i)),
                   static_cast<MFEMProblem &>(getFromMultiApp()->appProblemBase(i)));
          transfers_done++;
        }
      }
    }
    if (!transfers_done)
      mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp "
                 "per multiapp involved on each rank");
  }
}

void
MultiAppMFEMGeneralFieldTransfer::checkSiblingsTransferSupported() const
{
  // Check that we are in the supported configuration: same number of source and target apps
  // The allocation of the child apps on the processors must be the same
  if (getFromMultiApp()->numGlobalApps() == getToMultiApp()->numGlobalApps())
  {
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) + getToMultiApp()->hasLocalApp(i) == 1)
        mooseError("Child application allocation on parallel processes must be the same to support "
                   "siblings variable field copy transfer");
  }
  else
    mooseError("Number of source and target child apps must match for siblings transfer");
}

#endif
