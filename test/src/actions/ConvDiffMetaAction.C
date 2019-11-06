//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvDiffMetaAction.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"
#include "MooseApp.h"
#include "FEProblem.h"

#include "libmesh/vector_value.h"

registerMooseAction("MooseTestApp", ConvDiffMetaAction, "meta_action");

InputParameters
ConvDiffMetaAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "variables", "The names of the convection and diffusion variables in the simulation");

  return params;
}

ConvDiffMetaAction::ConvDiffMetaAction(const InputParameters & params) : Action(params) {}

void
ConvDiffMetaAction::act()
{
  std::shared_ptr<Action> action;
  std::shared_ptr<MooseObjectAction> moose_object_action;

  std::vector<NonlinearVariableName> variables =
      getParam<std::vector<NonlinearVariableName>>("variables");

  /**
   * We need to manually setup our Convection-Diffusion and Diffusion variables on our two
   * variables we are expecting from the input file.  Much of the syntax below is hidden by the
   * parser system but we have to set things up ourselves this time.
   */

  // Do some error checking
  mooseAssert(variables.size() == 2, "Expected 2 variables");

  //*******************************************//
  //**************** Variables ****************//
  //*******************************************//
  InputParameters variable_params = _action_factory.getValidParams("AddVariableAction");
  variable_params.set<ActionWarehouse *>("awh") = &_awh;

  // Create and Add First Variable Action
  action = _action_factory.create("AddVariableAction", variables[0], variable_params);
  _awh.addActionBlock(action);

  // Create and Add Second Variable Action
  action = _action_factory.create("AddVariableAction", variables[1], variable_params);
  _awh.addActionBlock(action);

  //*******************************************//
  //**************** Kernels ******************//
  //*******************************************//
  InputParameters action_params = _action_factory.getValidParams("AddKernelAction");
  action_params.set<ActionWarehouse *>("awh") = &_awh;

  // Setup our Diffusion Kernel on the "u" variable
  action_params.set<std::string>("type") = "Diffusion";
  action = _action_factory.create("AddKernelAction", "diff_u", action_params);
  moose_object_action = MooseSharedNamespace::dynamic_pointer_cast<MooseObjectAction>(action);
  mooseAssert(moose_object_action.get(), "Dynamic Cast failed");
  {
    InputParameters & params = moose_object_action->getObjectParams();
    params.set<NonlinearVariableName>("variable") = variables[0];
    // add it to the warehouse
    _awh.addActionBlock(action);
  }

  // Setup our Diffusion Kernel on the "v" variable
  action = _action_factory.create("AddKernelAction", "diff_v", action_params);

  moose_object_action = MooseSharedNamespace::dynamic_pointer_cast<MooseObjectAction>(action);
  mooseAssert(moose_object_action.get(), "Dynamic Cast failed");
  {
    InputParameters & params = moose_object_action->getObjectParams();
    params.set<NonlinearVariableName>("variable") = variables[1];
    // add it to the warehouse
    _awh.addActionBlock(action);
  }

  // Setup our Convection Kernel on the "u" variable coupled to the diffusion variable "v"
  action_params.set<std::string>("type") = "Convection";
  action = _action_factory.create("AddKernelAction", "conv_u", action_params);
  moose_object_action = MooseSharedNamespace::dynamic_pointer_cast<MooseObjectAction>(action);
  mooseAssert(moose_object_action.get(), "Dynamic Cast failed");
  {
    std::vector<std::string> vel_vec_variable;
    InputParameters & params = moose_object_action->getObjectParams();
    params.set<NonlinearVariableName>("variable") = variables[0];
    vel_vec_variable.push_back(variables[1]);
    params.set<std::vector<std::string>>("some_variable") = vel_vec_variable;

    params.set<RealVectorValue>("velocity") = RealVectorValue(0, 0, 0);
    // add it to the warehouse
    _awh.addActionBlock(action);
  }
}
