#include "ThermalContactAuxKernelsAction.h"
#include "Factory.h"
#include "FEProblem.h"

template<>
InputParameters validParams<ThermalContactAuxKernelsAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<std::string>("type", "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addParam<std::string>("conductivity_name", "thermal_conductivity", "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");
  return params;
}

ThermalContactAuxKernelsAction::ThermalContactAuxKernelsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
ThermalContactAuxKernelsAction::act()
{
  if (getParam<std::string>("type") == "GapHeatTransferLWR")
  {
    InputParameters params = _factory.getValidParams("MaterialRealAux");
    params.set<AuxVariableName>("variable") = "conductivity_"+getParam<NonlinearVariableName>("variable");
    params.set<std::string>("property") = getParam<std::string>("conductivity_name");
    // For efficiency, run this at the end of each step
    params.set<MooseEnum>("execute_on") = "timestep";

    _problem->addAuxKernel("MaterialRealAux",
        "conductivity_"+getParam<NonlinearVariableName>("variable"),
        params);
  }
}
