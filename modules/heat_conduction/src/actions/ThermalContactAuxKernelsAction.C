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
  params.addParam<std::string>("conductivity_master_name", "thermal_conductivity", "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");
  params.addParam<bool>("quadrature", false, "Whether or not to use quadrature point based gap heat transfer");
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  return params;
}

ThermalContactAuxKernelsAction::ThermalContactAuxKernelsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
ThermalContactAuxKernelsAction::act()
{
  const std::string cond_name = getParam<std::string>("conductivity_name");
  const std::string cond_master_name = getParam<std::string>("conductivity_master_name");
  bool different = (cond_name != cond_master_name);
  std::string slave("");
  std::string master("");
  if (different)
  {
    slave = "slave_";
    master = "master_";
  }

  if (getParam<std::string>("type") == "GapHeatTransferLWR")
  {
    InputParameters params = _factory.getValidParams("MaterialRealAux");
    // For efficiency, run this at the end of each step
    params.set<MooseEnum>("execute_on") = "timestep";
    params.set<std::string>("property") = cond_name;
    params.set<AuxVariableName>("variable") = "conductivity_"+slave+getParam<NonlinearVariableName>("variable");

    std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("slave"));
    params.set<std::vector<BoundaryName> >("boundary") = bnds;

    _problem->addAuxKernel("MaterialRealAux",
                           "conductivity_slave_"+getParam<NonlinearVariableName>("variable"),
                           params);

    {
      params.set<std::string>("property") = cond_master_name;
      params.set<AuxVariableName>("variable") = "conductivity_"+master+getParam<NonlinearVariableName>("variable");

      std::vector<BoundaryName> bnds(1, getParam<BoundaryName>("master"));
      params.set<std::vector<BoundaryName> >("boundary") = bnds;

      _problem->addAuxKernel("MaterialRealAux",
                             "conductivity_master_"+getParam<NonlinearVariableName>("variable"),
                             params);
    }
  }
}
