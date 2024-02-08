//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppCopyTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"

#include "libmesh/id_types.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", MultiAppCopyTransfer);

InputParameters
MultiAppCopyTransfer::validParams()
{
  InputParameters params = MultiAppDofCopyTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "The variable to transfer from.");

  params.addClassDescription(
      "Copies variables (nonlinear and auxiliary) between multiapps that have identical meshes.");
  return params;
}

MultiAppCopyTransfer::MultiAppCopyTransfer(const InputParameters & parameters)
  : MultiAppDofCopyTransfer(parameters),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variable"))
{
  /* Right now, most of derived transfers support one variable only */
  _to_var_name = _to_var_names.size() ? _to_var_names[0] : "INVALID";
  _from_var_name = _from_var_names.size() ? _from_var_names[0] : "INVALID";
}

void
MultiAppCopyTransfer::execute()
{
  TIME_SECTION("MultiAppCopyTransfer::execute()", 5, "Copies variables");

  if (_current_direction == TO_MULTIAPP)
  {
    FEProblemBase & from_problem = getToMultiApp()->problemBase();
    for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      if (getToMultiApp()->hasLocalApp(i))
        transfer(getToMultiApp()->appProblemBase(i), from_problem);
  }

  else if (_current_direction == FROM_MULTIAPP)
  {
    FEProblemBase & to_problem = getFromMultiApp()->problemBase();
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
      if (getFromMultiApp()->hasLocalApp(i))
        transfer(to_problem, getFromMultiApp()->appProblemBase(i));
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
          transfer(getToMultiApp()->appProblemBase(i), getFromMultiApp()->appProblemBase(i));
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
MultiAppCopyTransfer::checkSiblingsTransferSupported() const
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
