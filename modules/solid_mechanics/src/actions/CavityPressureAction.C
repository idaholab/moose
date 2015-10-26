/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CavityPressureAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<CavityPressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");

  params.addParam<std::vector<AuxVariableName> >("save_in_disp_x", "The save_in variables for x displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_y", "The save_in variables for y displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_z", "The save_in variables for z displacement");

  params.addParam<std::string>("output", "The name to use for the plenum pressure value.");

  return params;
}

CavityPressureAction::CavityPressureAction(const InputParameters & params) :
  Action(params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary")),
  _disp_x(getParam<NonlinearVariableName>("disp_x")),
  _disp_y(getParam<NonlinearVariableName>("disp_y")),
  _disp_z(getParam<NonlinearVariableName>("disp_z")),

  _kernel_name("Pressure"),
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
CavityPressureAction::act()
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

  PostprocessorName ppname;
  if (isParamValid("output"))
  {
    ppname = getParam<std::string>("output");
  }
  else
  {
    ppname = _name;
  }

  std::vector<NonlinearVariableName> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  for (unsigned int i(0); i < dim; ++i)
  {
    std::stringstream name;
    name << _name;
    name << "_";
    name << i;

    InputParameters params = _factory.getValidParams(_kernel_name);

    params.set<std::vector<BoundaryName> >("boundary") = _boundary;

    params.set<PostprocessorName>("postprocessor") = ppname;

    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = vars[i];
    if (_has_save_in_vars[i])
    {
      params.set<std::vector<AuxVariableName> >("save_in") = _save_in_vars[i];
    }

    _problem->addBoundaryCondition(_kernel_name, name.str(), params);
  }
}
