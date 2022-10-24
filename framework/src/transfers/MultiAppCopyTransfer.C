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

registerMooseObject("MooseApp", MultiAppCopyTransfer);

InputParameters
MultiAppCopyTransfer::validParams()
{
  InputParameters params = MultiAppFieldTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "The variable to transfer from.");

  params.addClassDescription(
      "Copies variables (nonlinear and auxiliary) between multiapps that have identical meshes.");
  return params;
}

MultiAppCopyTransfer::MultiAppCopyTransfer(const InputParameters & parameters)
  : MultiAppFieldTransfer(parameters),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variable"))
{
  /* Right now, most of derived transfers support one variable only */
  _to_var_name = _to_var_names[0];
  _from_var_name = _from_var_names[0];
}

void
MultiAppCopyTransfer::initialSetup()
{
  MultiAppFieldTransfer::initialSetup();

  const FEProblemBase * from_problem;
  const FEProblemBase * to_problem;

  if (_current_direction == FROM_MULTIAPP)
  {
    // Subdomain and variable type information is shared on all subapps
    from_problem = &getFromMultiApp()->appProblemBase(0);
    to_problem = &getFromMultiApp()->problemBase();
  }
  else if (_current_direction == TO_MULTIAPP)
  {
    from_problem = &getToMultiApp()->problemBase();
    to_problem = &getToMultiApp()->appProblemBase(0);
  }
  else
  {
    from_problem = &getFromMultiApp()->appProblemBase(0);
    to_problem = &getToMultiApp()->appProblemBase(0);
  }

  // Forbid block restriction on nodal variables as currently not supported
  if (_from_blocks.size())
    for (auto & from_var : getFromVarNames())
      if (from_problem
              ->getVariable(
                  0, from_var, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY)
              .hasDoFsOnNodes())
        paramError("from_blocks", "Block restriction is not implemented for nodal variables");
  if (_to_blocks.size())
    for (auto & to_var : getToVarNames())
      if (to_problem
              ->getVariable(
                  0, to_var, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY)
              .hasDoFsOnNodes())
        paramError("to_blocks", "Block restriction is not implemented for nodal variables");
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
