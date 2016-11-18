/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ThermalContactAuxVarsAction.h"
#include "FEProblem.h"
#include "libmesh/string_to_enum.h"

#include "FEProblem.h"

#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<ThermalContactAuxVarsAction>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

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

ThermalContactAuxVarsAction::ThermalContactAuxVarsAction(const InputParameters & params) :
   Action(params)
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

  if (quadrature)
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

}

