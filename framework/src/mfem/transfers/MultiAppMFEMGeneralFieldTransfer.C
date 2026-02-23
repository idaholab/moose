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
    
    // Generate list of points where the grid function will be evaluated
    mfem::Vector vxyz;
    mfem::Ordering::Type point_ordering;    
    extractProjectionPoints(to_pfespace, vxyz, point_ordering);

    const int NE = to_pfespace.GetParMesh()->GetNE();
    const int nsp = to_pfespace.GetTypicalFE()->GetNodes().GetNPoints();
    const int dim = to_pfespace.GetParMesh()->Dimension();

    // Evaluate source grid function at target points
    const int nodes_cnt = vxyz.Size() / dim;
    const int to_gf_ncomp = to_gf.VectorDim();
    mfem::Vector interp_vals(nodes_cnt*to_gf_ncomp);
    _mfem_interpolator.SetDefaultInterpolationValue(std::numeric_limits<mfem::real_t>::infinity());
    _mfem_interpolator.Interpolate(*from_gf.ParFESpace()->GetParMesh(), vxyz, from_gf, interp_vals, point_ordering);
    
    projectValues(interp_vals, to_pfespace.GetOrdering(), to_gf);
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
