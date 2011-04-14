#include "ContactAction.h"

#include "MProblem.h"
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
  _model(getParam<std::string>("model"))
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

  // Create master objects
  InputParameters params = validParams<DiracKernel>();
  params.set<std::string>("model") = _model;
  params.set<unsigned int>("boundary") = _master;
  params.set<unsigned int>("slave") = _slave;
  params.addCoupledVar("disp_x", "The x displacement");
  params.set<std::vector<std::string> >("disp_x") = std::vector<std::string>(1, _disp_x);
  params.addCoupledVar("disp_y", "The y displacement");
  params.set<std::vector<std::string> >("disp_y") = std::vector<std::string>(1, _disp_y);
  params.addCoupledVar("disp_z", "The z displacement");
  if (dim == 3)
  {
    params.set<std::vector<std::string> >("disp_z") = std::vector<std::string>(1, _disp_z);
  }
  std::vector<std::string> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  params.set<bool>("use_displaced_mesh") = true;
  for (unsigned int i(0); i < dim; ++i)
  {
    params.set<unsigned int>("component") = i;
    params.set<std::string>("variable") = vars[i];
    std::stringstream name;
    name << _name;
    name << "_master_";
    name << i;
    _parser_handle._problem->addDiracKernel("ContactMaster", name.str(), params);
  }

  params.set<unsigned int>("boundary") = _slave;
  params.set<unsigned int>("master") = _master;
  params.set<Real>("penalty") = _penalty;
  for (unsigned int i(0); i < dim; ++i)
  {
    params.set<unsigned int>("component") = i;
    params.set<std::string>("variable") = vars[i];
    std::stringstream name;
    name << _name;
    name << "_slave_";
    name << i;
    _parser_handle._problem->addDiracKernel("SlaveConstraint", name.str(), params);
  }
}
