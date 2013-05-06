#include "NodalAreaAuxAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"

static unsigned int counter = 0;

template<>
InputParameters validParams<NodalAreaAuxAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<bool>("output_area",false,"Output the nodal area");
  return params;
}

NodalAreaAuxAction::NodalAreaAuxAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _slave(getParam<BoundaryName>("slave"))
{
}

void
NodalAreaAuxAction::act()
{
  std::string short_name(_name);
  // Chop off "Contact/"
  short_name.erase(0, 8);

  {
    InputParameters params = _factory.getValidParams("NodalAreaAux");

    // Extract global params
    _app.parser().extractParams(_name, params);

    params.set<std::vector<BoundaryName> >("boundary") = std::vector<BoundaryName>(1,_slave);
    params.set<AuxVariableName>("variable") = "nodal_area_"+short_name;
    params.set<UserObjectName>("nodal_area_object") = "nodal_area_object_" + Moose::stringify(counter);

    params.set<bool>("use_displaced_mesh") = true;

    std::stringstream name;
    name << short_name;
    name << "_contact_";
    name << counter++;
    if (getParam<bool>("output_area"))
    {
      _problem->addAuxBoundaryCondition("NodalAreaAux",
                                        name.str(),
                                        params);
    }
  }
}
