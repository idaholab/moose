#include "ThermalContactAuxVarsAction.h"
#include "FEProblem.h"
#include "libmesh/string_to_enum.h"

#include "FEProblem.h"

#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<ThermalContactAuxVarsAction>()
{
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<Action>();

  params.addRequiredParam<std::string>("type", "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("quadrature", false, "Whether or not to use quadrature point based gap heat transfer");

  params.addParam<std::string>("conductivity_name", "thermal_conductivity", "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");
  params.addParam<std::string>("conductivity_master_name", "thermal_conductivity", "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");
  return params;
}

ThermalContactAuxVarsAction::ThermalContactAuxVarsAction(const std::string & name, InputParameters params) :
   Action(name, params)
{
}

void
ThermalContactAuxVarsAction::act()
{
  /*
  [./gap_value]
    order = FIRST
    family = LAGRANGE
  [../]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
  */

  bool quadrature = getParam<bool>("quadrature");

  // We need to add variables only once per variable name.  However, we don't know how many unique variable
  //   names we will have.  So, we'll always add them.

  MooseEnum order = getParam<MooseEnum>("order");
  std::string family = "LAGRANGE";

  std::string penetration_var_name("penetration");

  if(quadrature)
  {
    order = "CONSTANT";
    family = "MONOMIAL";
    penetration_var_name = "qpoint_penetration";
  }

  _problem->addAuxVariable(penetration_var_name,
    FEType(Utility::string_to_enum<Order>(order),
           Utility::string_to_enum<FEFamily>(family)));
  _problem->addAuxVariable(getGapValueName(_pars),
    FEType(Utility::string_to_enum<Order>(order),
           Utility::string_to_enum<FEFamily>(family)));

  if (getParam<std::string>("type") == "GapHeatTransferLWR")
  {
    _problem->addAuxVariable(getGapConductivityName(_pars),
      FEType(Utility::string_to_enum<Order>(order),
             Utility::string_to_enum<FEFamily>(family)));

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

    // Now add constant conductivity variable
    _problem->addAuxVariable("conductivity_"+slave+getParam<NonlinearVariableName>("variable"),
      FEType(Utility::string_to_enum<Order>("CONSTANT"),
             Utility::string_to_enum<FEFamily>("MONOMIAL")));
    _problem->addAuxVariable("conductivity_"+master+getParam<NonlinearVariableName>("variable"),
      FEType(Utility::string_to_enum<Order>("CONSTANT"),
             Utility::string_to_enum<FEFamily>("MONOMIAL")));

  }
}
