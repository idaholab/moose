#include "PressureAction.h"

#include "Pressure.h"

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

  InputParameters params = validParams<Pressure>();
  params.set<std::vector<unsigned int> >("boundary") = _boundary;
  params.set<Real>("factor") = _factor;
  params.set<std::string>("function") = _function;

  std::vector<std::string> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  params.set<bool>("use_displaced_mesh") = true;
  for (unsigned int i(0); i < dim; ++i)
  {
    params.set<int>("component") = i;
    params.set<std::string>("variable") = vars[i];
    std::stringstream name;
    name << _name;
    name << "_";
    name << i;
    _problem->addBoundaryCondition(_kernel_name, name.str(), params);
  }

}
