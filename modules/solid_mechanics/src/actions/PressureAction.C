#include "PressureAction.h"

#include "ActionFactory.h"
#include "MooseObjectAction.h"

#include "MProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<PressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<std::string>("disp_x", "The x displacement");
  params.addRequiredParam<std::string>("disp_y", "The y displacement");
  params.addParam<std::string>("disp_z", "", "The z displacement");
  params.addParam<Real>("factor", 1.0, "The factor to use in computing the pressure");
  params.addParam<std::string>("function", "", "The function that describes the pressure");
  return params;
}

PressureAction::PressureAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _boundary(getParam<std::vector<unsigned int> >("boundary")),
  _disp_x(getParam<std::string>("disp_x")),
  _disp_y(getParam<std::string>("disp_y")),
  _disp_z(getParam<std::string>("disp_z")),
  _factor(getParam<Real>("factor")),
  _function(getParam<std::string>("function")),
  _kernel_name("Pressure")
{
}

void
PressureAction::act()
{
  // Determine number of dimensions
  unsigned int dim(2);
  if (_disp_z != "")
  {
    ++dim;
  }

  InputParameters action_params = ActionFactory::instance()->getValidParams("BCs/*");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");
  action_params.set<std::string>("type") = _kernel_name;
  std::vector<std::string> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  std::string short_name(_name);
  // Chop off "BCs/Pressure/"
  short_name.erase(0, 4+_kernel_name.size());
  for (unsigned int i(0); i < dim; ++i)
  {
    std::stringstream name;
    name << "BCs/";
    name << short_name;
    name << "_";
    name << i;
    Action *action = ActionFactory::instance()->create(name.str(), action_params);

    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getMooseObjectParams();

    params.set<std::vector<unsigned int> >("boundary") = _boundary;
    params.set<Real>("factor") = _factor;
    params.set<std::string>("function") = _function;

    params.set<bool>("use_displaced_mesh") = true;

    params.set<int>("component") = i;
    params.set<std::string>("variable") = vars[i];

    // add it to the warehouse
    Moose::action_warehouse.addActionBlock(action);
  }

}
