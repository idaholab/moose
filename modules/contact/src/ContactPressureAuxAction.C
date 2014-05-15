#include "ContactPressureAuxAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "Conversion.h"

static unsigned int counter = 0;

template<>
InputParameters validParams<ContactPressureAuxAction>()
{
  MooseEnum orders("FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<MooseEnum>("order", orders, "The finite element order: " + orders.getRawNames());
  return params;
}

ContactPressureAuxAction::ContactPressureAuxAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _master(getParam<BoundaryName>("master")),
  _slave(getParam<BoundaryName>("slave")),
  _order(getParam<MooseEnum>("order"))
{
}

void
ContactPressureAuxAction::act()
{
  if (!_problem->getDisplacedProblem())
  {
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the Mesh block.");
  }

  std::string short_name(_name);
  // Chop off "Contact/"
  short_name.erase(0, 8);

  {
    InputParameters params = _factory.getValidParams("ContactPressureAux");

    // Extract global params
    _app.parser().extractParams(_name, params);

    params.set<std::vector<BoundaryName> >("boundary") = std::vector<BoundaryName>(1,_slave);
    params.set<BoundaryName>("paired_boundary") = _master;
    params.set<AuxVariableName>("variable") = "contact_pressure";
    params.addRequiredCoupledVar("nodal_area", "The nodal area");
    params.set<std::vector<VariableName> >("nodal_area") = std::vector<VariableName>(1, "nodal_area_"+short_name);
    params.set<MooseEnum>("order") = _order;

    params.set<bool>("use_displaced_mesh") = true;

    std::stringstream name;
    name << short_name;
    name << "_contact_pressure_";
    name << counter++;

    params.set<MooseEnum>("execute_on") = "jacobian";
    _problem->addAuxKernel("ContactPressureAux", name.str(), params);

    params.set<MooseEnum>("execute_on") = "timestep";
    name << "_timestep";
    _problem->addAuxKernel("ContactPressureAux", name.str(), params);

    params.set<MooseEnum>("execute_on") = "timestep_begin";
    name << "_timestep_begin";
    _problem->addAuxKernel("ContactPressureAux", name.str(), params);
  }
}
