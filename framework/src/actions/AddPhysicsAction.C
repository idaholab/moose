//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPhysicsAction.h"
#include "FEProblem.h"
#include "PhysicsBase.h"

registerMooseAction("MooseApp", AddPhysicsAction, "add_physics");

// TODO
// - add a meta action that happens before add_physics and registers AddPhysicsAction to
// the ones actually used by each physics, on a per-object basis if possible? so no risk to addFV
// for both a FE and FV physics
// - make the add_variables for physics happen after the variables block

// To create the variables
registerMooseAction("MooseApp", AddPhysicsAction, "add_variable");
registerMooseAction("MooseApp", AddPhysicsAction, "add_ic");

// To form the equations
registerMooseAction("MooseApp", AddPhysicsAction, "add_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_nodal_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_fv_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_dirac_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_dg_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_scalar_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_interface_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_fv_ik");
registerMooseAction("MooseApp", AddPhysicsAction, "add_bc");
// registerMooseAction("MooseApp", AddPhysicsAction, "add_nodal_bc");
registerMooseAction("MooseApp", AddPhysicsAction, "add_fv_bc");
registerMooseAction("MooseApp", AddPhysicsAction, "add_periodic_bc");

// Misc
registerMooseAction("MooseApp", AddPhysicsAction, "add_function");
registerMooseAction("MooseApp", AddPhysicsAction, "add_user_object");

// To help form part of the equations or to output spatially dependent quantities
// registerMooseAction("MooseApp", AddPhysicsAction, "add_aux_variable");
registerMooseAction("MooseApp", AddPhysicsAction, "add_aux_kernel");
registerMooseAction("MooseApp", AddPhysicsAction, "add_material");

// To output, for example solve-related quantities (iteration numbers etc)
registerMooseAction("MooseApp", AddPhysicsAction, "add_vector_postprocessor");
registerMooseAction("MooseApp", AddPhysicsAction, "add_postprocessor");
registerMooseAction("MooseApp", AddPhysicsAction, "add_reporter");
registerMooseAction("MooseApp", AddPhysicsAction, "add_output");

// To solve the equations
registerMooseAction("MooseApp", AddPhysicsAction, "add_preconditioning");
registerMooseAction("MooseApp", AddPhysicsAction, "setup_executioner");
registerMooseAction("MooseApp", AddPhysicsAction, "add_executor");

InputParameters
AddPhysicsAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Physics object to the simulation.");
  return params;
}

AddPhysicsAction::AddPhysicsAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddPhysicsAction::act()
{
  // Build the physics object and keep a pointer of it
  if (_current_task == "add_physics")
  {
    _problem->addPhysics(_type, _name, _moose_object_pars);
    _physics = _problem->getPhysics(_name);

    // Retrieve additional actions from the physics

    // TODO: Find a way to de-register a physics that does NOT use one of these actions
  }
  else
    mooseAssert(_physics, "Physics should have been created as it must be the first action to run");

  if (_current_task == "add_variable")
    _physics->PhysicsBase::addNonlinearVariables();
  // else if (_current_task == "add_ic")
  //   for (const auto [type, name, params] : getInfo(_physics->getInitialConditions()))
  //     _problem->addBoundaryCondition(type, name, params);
  else if (_current_task == "add_kernel")
    _physics->PhysicsBase::addFEKernels();
  // else if (_current_task == "add_nodal_kernel")
  //   for (const auto [type, name, params] : getInfo(_physics->getNodalKernels()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_fv_kernel")
  //   for (const auto [type, name, params] : getInfo(_physics->getFVKernels()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_dirac_kernel")
  //   for (const auto [type, name, params] : getInfo(_physics->getDiracKernels()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_dg_kernel")
  //   for (const auto [type, name, params] : getInfo(_physics->getDGKernels()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_scalar_kernel")
  //   for (const auto [type, name, params] : getInfo(_physics->getScalarKernels()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_interface_kernel")
  //   for (const auto [type, name, params] : getInfo(_physics->getInterfaceKernels()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_fv_ik")
  //   for (const auto [type, name, params] : getInfo(_physics->getFVInterfaceKernels()))
  //     _problem->addKernel(type, name, params);
  else if (_current_task == "add_bc")
    _physics->addFEBCs();

  // else if (_current_task == "add_nodal_bc")
  //   for (const auto [type, name, params] : getInfo(_physics->getNodalBCs()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_fv_bc")
  //   for (const auto [type, name, params] : getInfo(_physics->getFVBCs()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_periodic_bc")
  //   for (const auto [type, name, params] : getInfo(_physics->getPeriodicBCs()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_function")
  //   for (const auto [type, name, params] : getInfo(_physics->getFunctions()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_user_object")
  //   for (const auto [type, name, params] : getInfo(_physics->getUserObjects()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_aux_variable")
  //   for (const auto [type, name, params] : getInfo(_physics->getAuxiliaryVariables()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_aux_kernel")
  //   for (const auto [type, name, params] : getInfo(_physics->getAuxiliaryKernels()))
  //     _problem->addKernel(type, name, params);
  // // else if (_current_task == "add_postprocessor")
  // //   for (const auto [type, name, params] : getInfo(_physics->getPostprocessors()))
  // //     _problem->addKernel(type, name, params);
  // // else if (_current_task == "add_vector_postprocessor")
  // //   for (const auto [type, name, params] : getInfo(_physics->getVectorPostprocessors()))
  // //     _problem->addKernel(type, name, params);
  // // else if (_current_task == "add_reporter")
  // //   for (const auto [type, name, params] : getInfo(_physics->getReporters()))
  // //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_output")
  //   for (const auto [type, name, params] : getInfo(_physics->getOutputs()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_preconditioning")
  //   for (const auto [type, name, params] : getInfo(_physics->getPreconditioning()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_executioner")
  //   for (const auto [type, name, params] : getInfo(_physics->getExecutioners()))
  //     _problem->addKernel(type, name, params);
  // else if (_current_task == "add_executor")
  //   for (const auto [type, name, params] : getInfo(_physics->getExecutors()))
  //     _problem->addKernel(type, name, params);
}
