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
#include "Conversion.h"

template<>
InputParameters validParams<CavityPressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the pressure will be applied");
  params.addParam<std::vector<NonlinearVariableName> >("displacements", "The nonlinear displacement variables");
  params.addDeprecatedParam<NonlinearVariableName>("disp_x", "The x displacement", "Use displacements = 'disp_x disp_y disp_z'");
  params.addDeprecatedParam<NonlinearVariableName>("disp_y", "The y displacement", "Use displacements = 'disp_x disp_y disp_z'");
  params.addDeprecatedParam<NonlinearVariableName>("disp_z", "The z displacement", "Use displacements = 'disp_x disp_y disp_z'");
  params.addParam<std::vector<AuxVariableName> >("save_in", "Auxiliary variables to save the displacement residuals");
  params.addDeprecatedParam<std::vector<AuxVariableName> >("save_in_disp_x", "The save_in variables for x displacement", "Use save_in = 'save_in_disp_x save_in_disp_y save_in_disp_z'");
  params.addDeprecatedParam<std::vector<AuxVariableName> >("save_in_disp_y", "The save_in variables for y displacement", "Use save_in = 'save_in_disp_x save_in_disp_y save_in_disp_z'");
  params.addDeprecatedParam<std::vector<AuxVariableName> >("save_in_disp_z", "The save_in variables for z displacement", "Use save_in = 'save_in_disp_x save_in_disp_y save_in_disp_z'");
  params.addParam<std::string>("output", "The name to use for the plenum pressure value");
  params.addParam<bool>("use_displaced_mesh", true, "Whether to use displaced mesh in the boundary condition");
  return params;
}

CavityPressureAction::CavityPressureAction(const InputParameters & params) :
  Action(params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary"))
{
}

void
CavityPressureAction::act()
{
  std::vector<NonlinearVariableName> displacements;

  if (isParamValid("displacements"))
    displacements = getParam<std::vector<NonlinearVariableName> > ("displacements");
  else if (isParamValid("disp_x"))
  {
    displacements.push_back(getParam<NonlinearVariableName>("disp_x"));
    if (isParamValid("disp_y"))
    {
      displacements.push_back(getParam<NonlinearVariableName>("disp_y"));
      if (isParamValid("disp_z"))
        displacements.push_back(getParam<NonlinearVariableName>("disp_z"));
    }
  }
  else
    mooseError2("The input file should specify a string of displacement names, e.g. 'displacements = disp_x disp_y disp_z'; these names should match the Variable block names");

  unsigned int _ndisp = displacements.size();

  std::vector<std::vector<AuxVariableName> > save_in(_ndisp);

  if (isParamValid("save_in"))
  {
    std::vector<AuxVariableName> this_save_in = getParam<std::vector<AuxVariableName> >("save_in");
    for (unsigned int i = 0; i < _ndisp; ++i)
      save_in[i].push_back(this_save_in[i]);
  }
  else if (isParamValid("save_in_disp_x"))
    save_in[0] = getParam<std::vector<AuxVariableName> >("save_in_disp_x");
  else if (isParamValid("save_in_disp_y"))
    save_in[1] = getParam<std::vector<AuxVariableName> >("save_in_disp_y");
  else if (isParamValid("save_in_disp_z"))
    save_in[2] = getParam<std::vector<AuxVariableName> >("save_in_disp_z");

  if (isParamValid("save_in") && save_in.size() != _ndisp)
    mooseError2("Number of save_in variables should equal to the number of displacement variables ", _ndisp);

  PostprocessorName ppname;
  if (isParamValid("output"))
    ppname = getParam<std::string>("output");
  else
    ppname = _name;

  InputParameters params = _factory.getValidParams("Pressure");
  params.set<std::vector<BoundaryName> >("boundary") = _boundary;
  params.set<PostprocessorName>("postprocessor") = ppname;
  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    std::string name = _name + "_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];
    params.set<std::vector<AuxVariableName> >("save_in") = save_in[i];

    _problem->addBoundaryCondition("Pressure", name, params);
  }
}
