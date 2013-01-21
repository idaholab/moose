#include "PressureAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<PressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");
  params.addParam<Real>("factor", 1.0, "The factor to use in computing the pressure");
  params.addParam<FunctionName>("function", "", "The function that describes the pressure");
  return params;
}

PressureAction::PressureAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary")),
  _disp_x(getParam<NonlinearVariableName>("disp_x")),
  _disp_y(getParam<NonlinearVariableName>("disp_y")),
  _disp_z(getParam<NonlinearVariableName>("disp_z")),
  _factor(getParam<Real>("factor")),
  _function(getParam<FunctionName>("function")),
  _kernel_name("Pressure"),
  _use_displaced_mesh(true)
{
}

void
PressureAction::act()
{
  // Determine number of dimensions
  unsigned int dim(1);
  if (_disp_y != "")
  {
    ++dim;
  }
  if (_disp_z != "")
  {
    ++dim;
  }

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

    InputParameters params = Factory::instance()->getValidParams(_kernel_name);

    params.set<std::vector<BoundaryName> >("boundary") = _boundary;
    params.set<Real>("factor") = _factor;
    params.set<FunctionName>("function") = _function;

    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    params.set<int>("component") = i;
    params.set<NonlinearVariableName>("variable") = vars[i];

    _problem->addBoundaryCondition(_kernel_name, name.str(), params);
  }

}
