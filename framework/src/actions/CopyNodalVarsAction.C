/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "CopyNodalVarsAction.h"

#include "ActionWarehouse.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "NonlinearSystemBase.h"

#include <map>

template <>
InputParameters
validParams<CopyNodalVarsAction>()
{
  InputParameters params = validParams<Action>();
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

CopyNodalVarsAction::CopyNodalVarsAction(InputParameters params) : Action(params) {}

void
CopyNodalVarsAction::act()
{

  if (isParamValid("initial_from_file_var"))
  {
    SystemBase * system;

    if (_current_task == "check_copy_nodal_vars")
      _app.setFileRestart() = true;
    else
    {
      // Is this a NonlinearSystem variable or an AuxiliarySystem variable?
      if (_current_task == "copy_nodal_vars")
        system = &_problem->getNonlinearSystemBase();
      else
        system = &_problem->getAuxiliarySystem();

      system->addVariableToCopy(name(),
                                getParam<std::string>("initial_from_file_var"),
                                getParam<std::string>("initial_from_file_timestep"));
    }
  }
}
