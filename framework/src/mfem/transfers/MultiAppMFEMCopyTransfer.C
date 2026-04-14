//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiAppMFEMCopyTransfer.h"

registerMooseObject("MooseApp", MultiAppMFEMCopyTransfer);

InputParameters
MultiAppMFEMCopyTransfer::validParams()
{
  InputParameters params = MFEMMultiAppTransfer::validParams();
  params.addClassDescription("Copies variable values from one MFEM application to another");
  return params;
}

MultiAppMFEMCopyTransfer::MultiAppMFEMCopyTransfer(InputParameters const & params)
  : MFEMMultiAppTransfer(params)
{
  checkValidTransferProblemTypes<MFEMProblem, MFEMProblem>();
}

void
MultiAppMFEMCopyTransfer::transferVariables()
{
  auto getGF = [&](MFEMProblem & problem, const std::string & name) -> mfem::Vector &
  {
    if (problem.getProblemData().gridfunctions.Has(name))
      return *problem.getGridFunction(name);
    if (problem.getProblemData().cmplx_gridfunctions.Has(name))
      return *problem.getComplexGridFunction(name);
    mooseError("No real or complex variable named '", name, "' found.");
  };

  for (const auto v : make_range(numToVar()))
  {
    mfem::Vector & from_var = getGF(getActiveFromProblem(), getFromVarName(v));
    mfem::Vector & to_var = getGF(getActiveToProblem(), getToVarName(v));

    if (from_var.Size() != to_var.Size())
      mooseError("'", getFromVarName(v), "' and '", getToVarName(v), "' differ in no. of DoFs.");

    to_var = from_var;
  }
}

void
MultiAppMFEMCopyTransfer::checkSiblingsTransferSupported() const
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
