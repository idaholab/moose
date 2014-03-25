#include "CavityPressurePPAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<CavityPressurePPAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("output", "The name to use for the cavity pressure value.");
  params.addParam<std::string>("output_initial_moles", "The name to use when reporting the initial moles of gas.");

  return params;
}

CavityPressurePPAction::CavityPressurePPAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

void
CavityPressurePPAction::act()
{
  std::string short_name(_name);
  // Chop off "BCs/CavityPressure/"
  short_name.erase(0, 19);
  std::string uo_name = short_name + "UserObject";

  const std::string pp_name = "CavityPressurePostprocessor";

  InputParameters params = _factory.getValidParams(pp_name);

  params.set<MooseEnum>("execute_on") = "residual";

  params.set<UserObjectName>("cavity_pressure_uo") = uo_name;

  params.set<std::string>("quantity") = "cavity_pressure";
  if (isParamValid("output"))
  {
    _problem->addPostprocessor(pp_name, getParam<std::string>("output"), params);
  }
  else
  {
    _problem->addPostprocessor(pp_name, short_name, params);
  }

  if (isParamValid("output_initial_moles"))
  {
    params.set<std::string>("quantity") = "initial_moles";
    _problem->addPostprocessor(pp_name, getParam<std::string>("output_initial_moles"), params);
  }
}
