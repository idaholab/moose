/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ApplyCoupledVariablesTestAction.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"
#include "ActionWarehouse.h"

template <>
InputParameters
validParams<ApplyCoupledVariablesTestAction>()
{
  InputParameters params = validParams<Action>();
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
