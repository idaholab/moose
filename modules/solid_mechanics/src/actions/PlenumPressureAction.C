#include "PlenumPressureAction.h"

#include "ActionFactory.h"
#include "MooseObjectAction.h"

#include "MProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<PlenumPressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<std::string>("disp_x", "The x displacement");
  params.addRequiredParam<std::string>("disp_y", "The y displacement");
  params.addParam<std::string>("disp_z", "", "The z displacement");

  params.addParam<Real>("initial_pressure", 0, "The initial pressure in the plenum.  If not given, a zero initial pressure will be used.");
  params.addParam<std::string>("material_input", "", "The name of the postprocessor value that holds the amount of material injected into the plenum.");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used.");
  params.addRequiredParam<std::string>("temperature", "The name of the average temperature postprocessor value.");
  params.addRequiredParam<std::string>("volume", "The name of the internal volume postprocessor value.");
  params.addParam<Real>("startup_time", 0, "The amount of time during which the pressure will ramp from zero to its true value.");
  params.addParam<std::string>("output_initial_moles", "", "The reporting postprocessor to use for the initial moles of gas.");
  params.addParam<std::string>("output", "", "The reporting postprocessor to use for the plenum pressure value.");

  return params;
}

PlenumPressureAction::PlenumPressureAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _boundary(getParam<std::vector<unsigned int> >("boundary")),
  _disp_x(getParam<std::string>("disp_x")),
  _disp_y(getParam<std::string>("disp_y")),
  _disp_z(getParam<std::string>("disp_z")),
  _initial_pressure(getParam<Real>("initial_pressure")),
  _material_input(getParam<std::string>("material_input")),
  _R(getParam<Real>("R")),
  _temperature(getParam<std::string>("temperature")),
  _volume(getParam<std::string>("volume")),
  _startup_time(getParam<Real>("startup_time")),
  _output_initial_moles(getParam<std::string>("output_initial_moles")),
  _output(getParam<std::string>("output")),

  _kernel_name("PlenumPressure"),
  _use_displaced_mesh(true)
{
}

void
PlenumPressureAction::act()
{
  // Determine number of dimensions
  unsigned int dim(2);
  if (_disp_z != "")
  {
    ++dim;
  }

  InputParameters action_params = ActionFactory::instance()->getValidParams("AddBCAction");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
  action_params.set<std::string>("type") = _kernel_name;
  std::vector<std::string> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  std::string short_name(_name);
  // Chop off "BCs/PlenumPressure/"
  short_name.erase(0, 4+_kernel_name.size());
  for (unsigned int i(0); i < dim; ++i)
  {
    std::stringstream name;
    name << "BCs/";
    name << short_name;
    name << "_";
    name << i;
    action_params.set<std::string>("name") = name.str();
    Action *action = ActionFactory::instance()->create("AddBCAction", action_params);

    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getMooseObjectParams();

    params.set<std::vector<unsigned int> >("boundary") = _boundary;

    params.set<Real>("initial_pressure") = _initial_pressure;
    params.set<std::string>("material_input") = _material_input;
    params.set<Real>("R") = _R;
    params.set<std::string>("temperature") = _temperature;
    params.set<std::string>("volume") = _volume;
    params.set<Real>("startup_time") = _startup_time;
    params.set<std::string>("output_initial_moles") = _output_initial_moles;
    params.set<std::string>("output") = _output;

    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    params.set<int>("component") = i;
    params.set<std::string>("variable") = vars[i];

    // add it to the warehouse
    Moose::action_warehouse.addActionBlock(action);
  }

}
