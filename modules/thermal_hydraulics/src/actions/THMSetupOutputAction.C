//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSetupOutputAction.h"
#include "MooseApp.h"
#include "Factory.h"
#include "THMProblem.h"
#include "FlowModel.h"
#include "Output.h"
#include "Console.h"
#include "XDA.h"
#include "CSV.h"
#include "Tecplot.h"
#include "Exodus.h"
#include "InputParameterWarehouse.h"

registerMooseAction("ThermalHydraulicsApp", THMSetupOutputAction, "THM:setup_output");

InputParameters
THMSetupOutputAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addParam<bool>("disable_scalars_in_console",
                        true,
                        "Set to true to force 'execute_scalars_on = NONE' in Console, which "
                        "disables printing of all scalar variables.");

  params.addClassDescription("Sets up output for THM.");

  return params;
}

THMSetupOutputAction::THMSetupOutputAction(const InputParameters & params) : Action(params) {}

void
THMSetupOutputAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
  {
    for (auto && o : _app.getOutputWarehouse().getOutputs<Output>())
    {
      // Get a reference to the Output's InputParameters.  We have to
      // get it from the InputParameter Warehouse since we are going to
      // modify it.  Note that o->name() != "name" stored in the
      // object's InputParameters.  We need the latter to get the
      // InputParameters out of the Warehouse.
      // InputParameters & params =
      //   _app.getInputParameterWarehouse().getInputParameters(o->name(), /*tid=*/0);
      //
      // // If "hide" is available (AdvancedOutput) then add the hide_vars to it
      // if (params.have_parameter<std::vector<VariableName> >("hide"))
      // {
      //   std::vector<VariableName> hvars = params.get<std::vector<VariableName> >("hide");
      //   hvars.insert(hvars.end(), hide_vars.begin(), hide_vars.end());
      //   params.set<std::vector<VariableName> >("hide") = hvars;
      // }

      if (dynamic_cast<Console *>(o) != nullptr)
      {
        if (getParam<bool>("disable_scalars_in_console"))
        {
          // Do not output scalar variables on the screen.
          // CAUTION: there is no public API in MOOSE to control what gets outputted by an ouputter,
          // so we get the input parameters after the object was created and flip the flag there. At
          // this point it is still early enough, so that MOOSE won't notice.
          InputParameters & pars = const_cast<InputParameters &>(o->parameters());
          pars.set<ExecFlagEnum>("execute_scalars_on") = EXEC_NONE;
        }

        thm_problem->addScreenOutputter(o->name());
      }
      else if (dynamic_cast<XDA *>(o) != nullptr || dynamic_cast<CSV *>(o) != nullptr ||
               dynamic_cast<Tecplot *>(o) != nullptr || dynamic_cast<Exodus *>(o) != nullptr)
      {
        thm_problem->addFileOutputter(o->name());
      }
    }
  }
}
