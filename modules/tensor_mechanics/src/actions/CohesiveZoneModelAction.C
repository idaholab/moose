//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneModelAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("TensorMechanicsApp", CohesiveZoneModelAction, "add_czm_kernel");

template <>
InputParameters
validParams<CohesiveZoneModelAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set cohesive zone model kernels");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the pressure will be applied");

  params.addRequiredParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  return params;
}

CohesiveZoneModelAction::CohesiveZoneModelAction(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary"))
{
}

void
CohesiveZoneModelAction::act()
{
  std::string kernel_name = "CZMInterfaceKernel";

  std::vector<VariableName> displacements;
  if (isParamValid("displacements"))
    displacements = getParam<std::vector<VariableName>>("displacements");

  if (_current_task == "add_czm_kernel")
  {
    // Create pressure BCs
    for (unsigned int i = 0; i < displacements.size(); ++i)
    {
      // Create unique kernel name for each of the components
      std::string unique_kernel_name = kernel_name + "_" + _name + "_" + Moose::stringify(i);

      InputParameters params = _factory.getValidParams(kernel_name);

      params.set<bool>("use_displaced_mesh") = false;
      params.set<unsigned int>("disp_index") = i;
      params.set<NonlinearVariableName>("variable") = displacements[i];
      params.set<NonlinearVariableName>("neighbor_var") = displacements[i];
      params.set<std::vector<VariableName>>("displacements") = _displacements;
      params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;

      _problem->addInterfaceKernel(kernel_name, unique_kernel_name, params);
    }
  }
}
