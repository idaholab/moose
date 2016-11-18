/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CavityPressurePPAction.h"

#include "Factory.h"
#include "FEProblemBase.h"
#include "Parser.h"

template<>
InputParameters validParams<CavityPressurePPAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("output", "The name to use for the cavity pressure value");
  params.addParam<std::string>("output_initial_moles", "The name to use when reporting the initial moles of gas");
  return params;
}

CavityPressurePPAction::CavityPressurePPAction(InputParameters params) :
  Action(params)
{
}

void
CavityPressurePPAction::act()
{
  std::string uo_name = _name + "UserObject";

  InputParameters params = _factory.getValidParams("CavityPressurePostprocessor");
  params.set<MultiMooseEnum>("execute_on") = "linear";
  params.set<UserObjectName>("cavity_pressure_uo") = uo_name;
  params.set<std::string>("quantity") = "cavity_pressure";

  if (isParamValid("output"))
    _problem->addPostprocessor("CavityPressurePostprocessor", getParam<std::string>("output"), params);
  else
    _problem->addPostprocessor("CavityPressurePostprocessor", _name, params);

  if (isParamValid("output_initial_moles"))
  {
    params.set<std::string>("quantity") = "initial_moles";
    _problem->addPostprocessor("CavityPressurePostprocessor", getParam<std::string>("output_initial_moles"), params);
  }
}
