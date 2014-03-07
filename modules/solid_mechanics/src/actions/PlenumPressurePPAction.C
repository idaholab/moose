#include "PlenumPressurePPAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<PlenumPressurePPAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("output", "The name to use for the plenum pressure value.");
  params.addParam<std::string>("output_initial_moles", "The name to use when reporting the initial moles of gas.");

  return params;
}

PlenumPressurePPAction::PlenumPressurePPAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

void
PlenumPressurePPAction::act()
{
  std::string short_name(_name);
  // Chop off "BCs/PlenumPressure/"
  short_name.erase(0, 19);
  std::string uo_name = short_name + "UserObject";

  const std::string pp_name = "PlenumPressurePostprocessor";

  InputParameters params = _factory.getValidParams(pp_name);

  params.set<MooseEnum>("execute_on") = "residual";

  params.set<UserObjectName>("plenum_pressure_uo") = uo_name;

  params.set<std::string>("quantity") = "plenum_pressure";
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
