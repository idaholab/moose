//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ApplyCoupledVariablesTestAction.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"
#include "ActionWarehouse.h"

registerMooseAction("MooseTestApp", ApplyCoupledVariablesTestAction, "meta_action");

InputParameters
ApplyCoupledVariablesTestAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addCustomTypeParam("coef", 0.0, "CoefficientType", "The coefficient of diffusion");
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this Kernel operates on");
  return params;
}

ApplyCoupledVariablesTestAction::ApplyCoupledVariablesTestAction(const InputParameters & params)
  : Action(params)
{
}

ApplyCoupledVariablesTestAction::~ApplyCoupledVariablesTestAction() {}

void
ApplyCoupledVariablesTestAction::act()
{
  // Set the 'type =' parameters for the desired object
  InputParameters action_params = _action_factory.getValidParams("AddKernelAction");
  action_params.set<std::string>("type") = "CoefDiffusion";
  action_params.set<ActionWarehouse *>("awh") = &_awh;

  // Create the action
  std::string long_name = "Kernels/_coef_diffusion";
  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create("AddKernelAction", long_name, action_params));

  // Apply the parameters from the this action to the object being created
  action->getObjectParams().applyParameters(_pars);

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}
