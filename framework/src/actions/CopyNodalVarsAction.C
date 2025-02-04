//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CopyNodalVarsAction.h"

#include "ActionWarehouse.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "NonlinearSystemBase.h"

#include <map>

registerMooseAction("MooseApp", CopyNodalVarsAction, "check_copy_nodal_vars");

registerMooseAction("MooseApp", CopyNodalVarsAction, "copy_nodal_vars");

registerMooseAction("MooseApp", CopyNodalVarsAction, "copy_nodal_aux_vars");

InputParameters
CopyNodalVarsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Copies variable information from a file.");
  params.addParam<std::string>(
      "initial_from_file_timestep",
      "LATEST",
      "Gives the timestep (or \"LATEST\") for which to read a solution from a file "
      "for a given variable. (Default: LATEST)");
  params.addParam<std::string>(
      "initial_from_file_var",
      "Gives the name of a variable for which to read an initial condition from a mesh file");

  params.addParamNamesToGroup("initial_from_file_timestep initial_from_file_var",
                              "Initial From File");

  return params;
}

CopyNodalVarsAction::CopyNodalVarsAction(const InputParameters & params) : Action(params) {}

void
CopyNodalVarsAction::act()
{

  if (isParamValid("initial_from_file_var"))
  {
    if (_current_task == "check_copy_nodal_vars")
      _app.setExodusFileRestart(true);
    else
    {
      SystemBase * system;
      // Is this a NonlinearSystem variable or an AuxiliarySystem variable?
      if (_current_task == "copy_nodal_vars")
      {
        // This iterates through each nonlinear system and finds which one the current variable
        // needs to be copied to
        system = &_problem->getSolverSystem(/*sys_num=*/0);
        for (unsigned int i = 0; i < _problem->numSolverSystems(); i++)
          if (_problem->getSolverSystem(i).hasVariable(name()))
            system = &_problem->getSolverSystem(i);
      }
      else
        system = &_problem->getAuxiliarySystem();

      system->addVariableToCopy(name(),
                                getParam<std::string>("initial_from_file_var"),
                                getParam<std::string>("initial_from_file_timestep"));
    }
  }
}
