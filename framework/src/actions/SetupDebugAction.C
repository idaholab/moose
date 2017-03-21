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

#include "SetupDebugAction.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "Factory.h"
#include "Output.h"
#include "MooseApp.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"

template <>
InputParameters
validParams<SetupDebugAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<unsigned int>(
      "show_top_residuals", 0, "The number of top residuals to print out (0 = no output)");
  params.addParam<bool>(
      "show_var_residual_norms",
      false,
      "Print the residual norms of the individual solution variables at each nonlinear iteration");
  params.addParam<bool>("show_actions", false, "Print out the actions being executed");
  params.addParam<bool>(
      "show_parser", false, "Shows parser block extraction and debugging information");
  params.addParam<bool>(
      "show_material_props",
      false,
      "Print out the material properties supplied for each block, face, neighbor, and/or sideset");
  return params;
}

SetupDebugAction::SetupDebugAction(InputParameters parameters)
  : Action(parameters), _action_params(_action_factory.getValidParams("AddOutputAction"))
{
  _awh.showActions(getParam<bool>("show_actions"));
  _awh.showParser(getParam<bool>("show_parser"));

  // Set the ActionWarehouse pointer in the parameters that will be passed to the actions created
  // with this action
  _action_params.set<ActionWarehouse *>("awh") = &_awh;
}

void
SetupDebugAction::act()
{
  // Material properties
  if (_pars.get<bool>("show_material_props"))
    createOutputAction("MaterialPropertyDebugOutput", "_moose_material_property_debug_output");

  // Variable residusl norms
  if (_pars.get<bool>("show_var_residual_norms"))
    createOutputAction("VariableResidualNormsDebugOutput",
                       "_moose_variable_residual_norms_debug_output");

  // Top residuals
  if (_pars.get<unsigned int>("show_top_residuals") > 0)
  {
    MooseObjectAction * action =
        createOutputAction("TopResidualDebugOutput", "_moose_top_residual_debug_output");
    action->getObjectParams().set<unsigned int>("num_residuals") =
        _pars.get<unsigned int>("show_top_residuals");
  }
}

MooseObjectAction *
SetupDebugAction::createOutputAction(const std::string & type, const std::string & name)
{
  // Set the 'type =' parameters for the desired object
  _action_params.set<std::string>("type") = type;

  // Create the action
  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create("AddOutputAction", name, _action_params));

  // Set the object parameters
  InputParameters & object_params = action->getObjectParams();
  object_params.set<bool>("_built_by_moose") = true;

  // Add the action to the warehouse
  _awh.addActionBlock(action);

  // Return the pointer to the action
  return action.get();
}
