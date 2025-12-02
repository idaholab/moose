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
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"

registerMooseObject("MooseApp", MultiAppMFEMCopyTransfer);

InputParameters
MultiAppMFEMCopyTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "AuxVariable to store transferred value in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "Variable to transfer from");
  params.addClassDescription("Copies variable values from one MFEM application to another");
  return params;
}

MultiAppMFEMCopyTransfer::MultiAppMFEMCopyTransfer(InputParameters const & params)
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

//
void
MultiAppMFEMCopyTransfer::transfer(MFEMProblem & to_problem, MFEMProblem & from_problem)
{
  // Redundant as source name is required?
  if (!numToVar())
    mooseError("No transferred variables were specified, neither programmatically or through the "
               "'source_variable' parameter");
  if (numToVar() != numFromVar())
    mooseError("Number of variables transferred must be same in both systems.");
  for (unsigned v = 0; v < numToVar(); ++v)
  {
    const auto & name_from = getFromVarName(v);
    const auto & name_to   = getToVarName(v);
    // TODO: Probably need more checking here to make sure the variables are
    // copyable - as per the MultiAppDofCopyTransfer
    auto & from_data = from_problem.getProblemData();
    auto & to_data   = to_problem.getProblemData();
    // ===== REAL to REAL ==================================================
    if (from_data.gridfunctions.Has(name_from))
    {
      if (!to_data.gridfunctions.Has(name_to))
        mooseError("MultiAppMFEMCopyTransfer: trying to copy real field '",
                   name_from, "' into non-real target field '", name_to, "'");

      auto & from_var = from_data.gridfunctions.GetRef(name_from);
      auto & to_var   = to_data.gridfunctions.GetRef(name_to);

      to_var = from_var;
      continue;
    }
    // ===== COMPLEX to COMPLEX =============================================
    if (from_data.cmplx_gridfunctions.Has(name_from))
    {
      if (!to_data.cmplx_gridfunctions.Has(name_to))
        mooseError("MultiAppMFEMCopyTransfer: trying to copy complex field '",
                   name_from, "' into non-complex target field '", name_to, "'");

      auto & from_c = from_data.cmplx_gridfunctions.GetRef(name_from);
      auto & to_c   = to_data.cmplx_gridfunctions.GetRef(name_to);

      // Copy real part
      to_c.real() = from_c.real();
      // Copy imaginary part
      to_c.imag() = from_c.imag();

      continue;
    }
    mooseError("MultiAppMFEMCopyTransfer: field '", name_from,
               "' not found as real or complex in source problem.");
  }
}

void
MultiAppMFEMCopyTransfer::execute()
{
  TIME_SECTION("MultiAppMFEMCopyTransfer::execute", 5, "Copies variables");
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
