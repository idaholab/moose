/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PressureActionTM.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<PressureActionTM>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up PressureTM boundary conditions");

  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");

  params.addParam<std::vector<AuxVariableName> >("save_in_disp_x", "The save_in variables for x displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_y", "The save_in variables for y displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_z", "The save_in variables for z displacement");

  params.addParam<Real>("factor", 1.0, "The factor to use in computing the pressure");
  params.addParam<FunctionName>("function", "The function that describes the pressure");
  params.addParam<PostprocessorName>("postprocessor", "", "The postprocessor that describes the pressure");
  return params;
}

PressureActionTM::PressureActionTM(const std::string & name, InputParameters params) :
  Action(name, params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary")),
  _disp_x(getParam<NonlinearVariableName>("disp_x")),
  _disp_y(getParam<NonlinearVariableName>("disp_y")),
  _disp_z(getParam<NonlinearVariableName>("disp_z")),
  _factor(getParam<Real>("factor")),
  _postprocessor(getParam<PostprocessorName>("postprocessor")),
  _kernel_name("PressureTM"),
  _use_displaced_mesh(true)
{
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_x"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_y"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_z"));

  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_x"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_y"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_z"));
}

void
PressureActionTM::act()
{
  // Determine number of dimensions
  unsigned int dim = 1;

  if (_disp_y != "")
    ++dim;

  if (_disp_z != "")
    ++dim;

  std::vector<NonlinearVariableName> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  std::string short_name(_name);

  // Chop off "BCs/Pressure/"
  short_name.erase(0, 5+_kernel_name.size());

  //Create pressure BCs
  for (unsigned int i = 0; i < dim; ++i)
  {
    std::stringstream name;
    name << "BCs/";
    name << short_name;
    name << "_";
    name << i;

    InputParameters params = _factory.getValidParams(_kernel_name);

    params.set<std::vector<BoundaryName> >("boundary") = _boundary;
    params.set<Real>("factor") = _factor;
    if (isParamValid("function"))
      params.set<FunctionName>("function") = getParam<FunctionName>("function");

    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = vars[i];

    if (_has_save_in_vars[i])
      params.set<std::vector<AuxVariableName> >("save_in") = _save_in_vars[i];

    _problem->addBoundaryCondition(_kernel_name, name.str(), params);
  }
}
