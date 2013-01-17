#include "NodalAreaAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"

static unsigned int counter = 0;

template<>
InputParameters validParams<NodalAreaAction>()
{
  MooseEnum orders("FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<BoundaryName>("slave", "The slave surface");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  return params;
}

NodalAreaAction::NodalAreaAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

void
NodalAreaAction::act()
{

  InputParameters params = Factory::instance()->getValidParams("NodalArea");

  // Extract global params
  const std::string syntax = Moose::app->parser().getSyntaxByAction("NodalArea", "");
  Moose::app->parser().extractParams(syntax, params);

  params.set<std::vector<BoundaryName> >("boundary") = std::vector<BoundaryName>(1,getParam<BoundaryName>("slave"));
  params.set<std::vector<VariableName> >("variable") = std::vector<VariableName>(1, getParam<NonlinearVariableName>("disp_x"));

  params.set<MooseEnum>("execute_on") = "timestep_begin";
  params.set<bool>("use_displaced_mesh") = true;

  _problem->addUserObject("NodalArea",
                          "nodal_area_object_" + Moose::stringify(counter++),
                          params);
}
