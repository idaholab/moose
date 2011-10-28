#include "ContactAction.h"

#include "ActionFactory.h"
#include "MooseObjectAction.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<ContactAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<unsigned int>("master", "The master surface");
  params.addRequiredParam<unsigned int>("slave", "The slave surface");
  params.addRequiredParam<std::string>("disp_x", "The x displacement");
  params.addRequiredParam<std::string>("disp_y", "The y displacement");
  params.addParam<std::string>("disp_z", "", "The z displacement");
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");
  params.addParam<std::string>("order", "FIRST", "The finite element order");
  return params;
}

ContactAction::ContactAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _master(getParam<unsigned int>("master")),
  _slave(getParam<unsigned int>("slave")),
  _disp_x(getParam<std::string>("disp_x")),
  _disp_y(getParam<std::string>("disp_y")),
  _disp_z(getParam<std::string>("disp_z")),
  _penalty(getParam<Real>("penalty")),
  _model(getParam<std::string>("model")),
  _order(getParam<std::string>("order"))
{
}

void
ContactAction::act()
{
  // Determine number of dimensions
  unsigned int dim(2);
  if (_disp_z != "")
  {
    ++dim;
  }

  InputParameters action_params = ActionFactory::instance()->getValidParams("AddDiracKernelAction");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");

  // Create master objects
  action_params.set<std::string>("type") = "ContactMaster";
  std::vector<std::string> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  std::string short_name(_name);
  // Chop off "Contact/"
  short_name.erase(0, 8);
  for (unsigned int i(0); i < dim; ++i)
  {
    std::stringstream name;
    name << "DiracKernels/";
    name << short_name;
    name << "_master_";
    name << i;
    action_params.set<std::string>("name") = name.str();
    Action *action = ActionFactory::instance()->create("AddDiracKernelAction", action_params);

    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();

    params.set<std::string>("model") = _model;
    params.set<std::string>("order") = _order;
    params.set<unsigned int>("boundary") = _master;
    params.set<unsigned int>("slave") = _slave;
    params.set<Real>("penalty") = _penalty;
    params.addCoupledVar("disp_x", "The x displacement");
    params.set<std::vector<std::string> >("disp_x") = std::vector<std::string>(1, _disp_x);
    params.addCoupledVar("disp_y", "The y displacement");
    params.set<std::vector<std::string> >("disp_y") = std::vector<std::string>(1, _disp_y);
    params.addCoupledVar("disp_z", "The z displacement");
    if (dim == 3)
    {
      params.set<std::vector<std::string> >("disp_z") = std::vector<std::string>(1, _disp_z);
    }

    params.set<bool>("use_displaced_mesh") = true;
    params.set<unsigned int>("component") = i;
    params.set<std::string>("variable") = vars[i];

    // add it to the warehouse
    Moose::action_warehouse.addActionBlock(action);
  }

  // Create slave objects
  action_params.set<std::string>("type") = "SlaveConstraint";
  for (unsigned int i(0); i < dim; ++i)
  {
    std::stringstream name;
    name << "DiracKernels/";
    name << short_name;
    name << "_slave_";
    name << i;
    action_params.set<std::string>("name") = name.str();
    Action *action = ActionFactory::instance()->create("AddDiracKernelAction", action_params);

    MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(action);
    mooseAssert (moose_object_action, "Dynamic Cast failed");

    InputParameters & params = moose_object_action->getObjectParams();

    params.set<std::string>("model") = _model;
    params.set<std::string>("order") = _order;
    params.set<unsigned int>("boundary") = _slave;
    params.set<unsigned int>("master") = _master;
    params.set<Real>("penalty") = _penalty;
    params.addCoupledVar("disp_x", "The x displacement");
    params.set<std::vector<std::string> >("disp_x") = std::vector<std::string>(1, _disp_x);
    params.addCoupledVar("disp_y", "The y displacement");
    params.set<std::vector<std::string> >("disp_y") = std::vector<std::string>(1, _disp_y);
    params.addCoupledVar("disp_z", "The z displacement");
    if (dim == 3)
    {
      params.set<std::vector<std::string> >("disp_z") = std::vector<std::string>(1, _disp_z);
    }

    params.set<bool>("use_displaced_mesh") = true;
    params.set<unsigned int>("component") = i;
    params.set<std::string>("variable") = vars[i];

    // add it to the warehouse
    Moose::action_warehouse.addActionBlock(action);
  }

}
