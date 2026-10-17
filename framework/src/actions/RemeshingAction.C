//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RemeshingAction.h"

#include "FEProblem.h"
#include "Remeshing.h"

registerMooseAction("MooseApp", RemeshingAction, "setup_remeshing");

InputParameters
RemeshingAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action for turning on remeshing and defining its parameters.");
  params.addParam<bool>(
      "mesh_movement",
      false,
      "Whether a mesh smoother moves the mesh between remesh events. When this is false the "
      "pseudo-displacement accumulated since the last remesh is identically zero.");
  params.addParam<std::vector<VariableName>>(
      "displacements",
      {},
      "The displacement variables of the problem, when it has any. Setting them makes the criteria "
      "measure the displaced configuration and keeps the remeshing itself on the reference "
      "configuration.");
  params.addRangeCheckedParam<unsigned int>(
      "check_interval",
      1,
      "check_interval > 0",
      "The number of time steps between two evaluations of the criteria.");
  params.addParam<unsigned int>(
      "initial_remesh_cycles",
      0,
      "The largest number of remesh cycles performed on the initial condition, before the "
      "transient starts, each one re-projecting the initial conditions onto the mesh it produced. "
      "The cycles stop early once no criterion fires, and zero, the default, starts the transient "
      "on the mesh as it was built.");
  return params;
}

RemeshingAction::RemeshingAction(const InputParameters & params) : Action(params) {}

void
RemeshingAction::act()
{
  _problem->getRemeshing().init(getParam<bool>("mesh_movement"),
                                getParam<std::vector<VariableName>>("displacements"),
                                getParam<unsigned int>("check_interval"),
                                getParam<unsigned int>("initial_remesh_cycles"));
}
