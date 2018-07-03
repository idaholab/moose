#include "RELAP7SetupOutputAction.h"
#include "MooseApp.h"
#include "Factory.h"
#include "Simulation.h"
#include "FlowModel.h"
#include "Output.h"
#include "Console.h"
#include "XDA.h"
#include "CSV.h"
#include "Tecplot.h"
#include "Exodus.h"
#include "InputParameterWarehouse.h"

registerMooseAction("RELAP7App", RELAP7SetupOutputAction, "RELAP7:setup_output");

template <>
InputParameters
validParams<RELAP7SetupOutputAction>()
{
  InputParameters params = validParams<RELAP7Action>();
  return params;
}

RELAP7SetupOutputAction::RELAP7SetupOutputAction(InputParameters params) : RELAP7Action(params) {}

void
RELAP7SetupOutputAction::act()
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

    if (dynamic_cast<Exodus *>(o) != nullptr)
    {
      Exodus * exodus = dynamic_cast<Exodus *>(o);
      // Always use 3D output files for exodus
      exodus->setOutputDimension(3);
    }

    if (dynamic_cast<Console *>(o) != nullptr)
    {
      // Do not output scalar variables on the screen.
      // CAUTION: there is no public API in MOOSE to control what gets outputted by an ouputter, so
      // we get the input parameters after the object was
      // created and flip the flag there. At this point it is still early enough, so that MOOSE
      // won't notice.
      InputParameters & pars = const_cast<InputParameters &>(o->parameters());
      pars.set<ExecFlagEnum>("execute_scalars_on") = EXEC_NONE;

      _simulation.addScreenOutputter(o->name());
    }
    else if (dynamic_cast<XDA *>(o) != nullptr || dynamic_cast<CSV *>(o) != nullptr ||
             dynamic_cast<Tecplot *>(o) != nullptr || dynamic_cast<Exodus *>(o) != nullptr)
    {
      _simulation.addFileOutputter(o->name());
    }
  }
}
