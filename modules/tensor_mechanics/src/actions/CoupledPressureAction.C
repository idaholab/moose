//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledPressureAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("TensorMechanicsApp", CoupledPressureAction, "add_bc");

InputParameters
CoupledPressureAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up Coupled Pressure boundary conditions");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the pressure will be applied");

  params.addParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addParam<bool>("use_displaced_mesh", true, "Whether to use the displaced mesh.");
  params.addParam<std::vector<AuxVariableName>>("save_in_disp_x",
                                                "The save_in variables for x displacement");
  params.addParam<std::vector<AuxVariableName>>("save_in_disp_y",
                                                "The save_in variables for y displacement");
  params.addParam<std::vector<AuxVariableName>>("save_in_disp_z",
                                                "The save_in variables for z displacement");

  params.addParam<VariableName>("pressure", "The variable that contains the pressure");
  return params;
}

CoupledPressureAction::CoupledPressureAction(const InputParameters & params) : Action(params)
{
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName>>("save_in_disp_x"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName>>("save_in_disp_y"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName>>("save_in_disp_z"));

  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_x"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_y"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_z"));
}

void
CoupledPressureAction::act()
{
  const std::string kernel_name = "CoupledPressureBC";

  std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");

  // Create pressure BCs
  for (unsigned int i = 0; i < displacements.size(); ++i)
  {
    // Create unique kernel name for each of the components
    std::string unique_kernel_name = kernel_name + "_" + _name + "_" + Moose::stringify(i);

    InputParameters params = _factory.getValidParams(kernel_name);
    params.applySpecificParameters(parameters(), {"boundary"});
    params.set<std::vector<VariableName>>("pressure") = {getParam<VariableName>("pressure")};
    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = displacements[i];

    if (_has_save_in_vars[i])
      params.set<std::vector<AuxVariableName>>("save_in") = _save_in_vars[i];

    _problem->addBoundaryCondition(kernel_name, unique_kernel_name, params);
  }
}
