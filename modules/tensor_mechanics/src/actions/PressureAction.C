/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PressureAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template <>
InputParameters
validParams<PressureAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up Pressure boundary conditions");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the pressure will be applied");

  params.addParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "The z displacement");

  params.addParam<std::vector<NonlinearVariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addParam<std::vector<AuxVariableName>>("save_in_disp_x",
                                                "The save_in variables for x displacement");
  params.addParam<std::vector<AuxVariableName>>("save_in_disp_y",
                                                "The save_in variables for y displacement");
  params.addParam<std::vector<AuxVariableName>>("save_in_disp_z",
                                                "The save_in variables for z displacement");

  params.addParam<Real>("factor", 1.0, "The factor to use in computing the pressure");
  params.addParam<Real>("alpha", 0.0, "alpha parameter for HHT time integration");
  params.addParam<FunctionName>("function", "The function that describes the pressure");
  return params;
}

PressureAction::PressureAction(const InputParameters & params) : Action(params)
{
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName>>("save_in_disp_x"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName>>("save_in_disp_y"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName>>("save_in_disp_z"));

  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_x"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_y"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_z"));
}

void
PressureAction::act()
{
  const std::string kernel_name = "Pressure";

  std::vector<NonlinearVariableName> displacements;
  if (isParamValid("displacements"))
    displacements = getParam<std::vector<NonlinearVariableName>>("displacements");
  else
  {
    // Legacy parameter scheme for displacements
    if (!isParamValid("disp_x"))
      mooseError("Specify displacement variables using the `displacements` parameter.");
    displacements.push_back(getParam<NonlinearVariableName>("disp_x"));

    if (isParamValid("disp_y"))
    {
      displacements.push_back(getParam<NonlinearVariableName>("disp_y"));
      if (isParamValid("disp_z"))
        displacements.push_back(getParam<NonlinearVariableName>("disp_z"));
    }
  }

  // Create pressure BCs
  for (unsigned int i = 0; i < displacements.size(); ++i)
  {
    // Create unique kernel name for each of the components
    std::string unique_kernel_name = kernel_name + "_" + _name + "_" + Moose::stringify(i);

    InputParameters params = _factory.getValidParams(kernel_name);
    params.applyParameters(parameters());
    params.set<bool>("use_displaced_mesh") = true;
    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];

    if (_has_save_in_vars[i])
      params.set<std::vector<AuxVariableName>>("save_in") = _save_in_vars[i];

    _problem->addBoundaryCondition(kernel_name, unique_kernel_name, params);
  }
}
