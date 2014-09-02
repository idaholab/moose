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

template<>
InputParameters validParams<SetupDebugAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<unsigned int>("show_top_residuals", 0, "The number of top residuals to print out (0 = no output)");
  params.addParam<bool>("show_var_residual_norms", false, "Print the residual norms of the individual solution variables at each nonlinear iteration");
  params.addParam<bool>("show_actions", false, "Print out the actions being executed");
  params.addParam<bool>("show_parser", false, "Shows parser block extraction and debugging information");
  params.addParam<bool>("show_material_props", false, "Print out the material properties supplied for each block, face, neighbor, and/or sideset");
  return params;
}

SetupDebugAction::SetupDebugAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _top_residuals(getParam<unsigned int>("show_top_residuals")),
    _action_params(_action_factory.getValidParams("AddOutputAction"))
{
  _awh.showActions(getParam<bool>("show_actions"));
  _awh.showParser(getParam<bool>("show_parser"));

  // Set the ActionWarehouse pointer in the parameters that will be passed to the actions created with this action
  _action_params.set<ActionWarehouse *>("awh") = &_awh;
}

SetupDebugAction::~SetupDebugAction()
{
}

void
SetupDebugAction::act()
{
  // Flags debug outputting via the DebugOutput object
  bool show_var_residual_norms = _pars.get<bool>("show_var_residual_norms");
  bool show_material_props = _pars.get<bool>("show_material_props");

  // Create DebugOutput object
  if (show_var_residual_norms || show_material_props)
  {
    // Set the 'type =' parameters for the desired object
    _action_params.set<std::string>("type") = "DebugOutput";

    // Create the action
    MooseObjectAction * action = static_cast<MooseObjectAction *>(_action_factory.create("AddOutputAction", "Outputs/_moose_debug_output", _action_params));

    // Set the object parameters
    InputParameters & object_params = action->getObjectParams();
    object_params.set<bool>("_built_by_moose") = true;
    object_params.set<bool>("show_var_residual_norms") = show_var_residual_norms;
    object_params.set<bool>("show_material_props") = show_material_props;

    // Add the action to the warehouse
    _awh.addActionBlock(action);
  }

  // Enable MaterialProperty and top residual debugging
  if (_problem != NULL)
    _problem->setDebugTopResiduals(_top_residuals);
}
