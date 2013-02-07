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

#include "ConvDiffMetaAction.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"
#include "MooseApp.h"
#include "FEProblem.h"

#include "libmesh/vector_value.h"

template<>
InputParameters validParams<ConvDiffMetaAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName> >("variables", "The names of the convection and diffusion variables in the simulation");

  return params;
}

ConvDiffMetaAction::ConvDiffMetaAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
ConvDiffMetaAction::act()
{
  Action *action;
  MooseObjectAction *moose_object_action;

  std::vector<NonlinearVariableName> variables = getParam<std::vector<NonlinearVariableName> > ("variables");

//  std::cerr << "Acting on " << getParam<std::string>("built_by_action") << "\n\n";

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

//  for (unsigned int i=0; i<variable_params.size(); ++i)
//  {

  // Create and Add First Variable Action
  action = _action_factory.create("AddVariableAction", "Variables/" + variables[0], variable_params);
  _awh.addActionBlock(action);

  // Create and Add Second Variable Action
  action = _action_factory.create("AddVariableAction", "Variables/" + variables[1], variable_params);
  _awh.addActionBlock(action);
//  }


  //*******************************************//
  //**************** Kernels ******************//
  //*******************************************//
  InputParameters action_params = _action_factory.getValidParams("AddKernelAction");
  action_params.set<ActionWarehouse *>("awh") = &_awh;

  // Setup our Diffusion Kernel on the "u" variable
  action_params.set<std::string>("type") = "Diffusion";
  action = _action_factory.create("AddKernelAction", "Kernels/diff_u", action_params);
  moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");
  {
    InputParameters & params = moose_object_action->getObjectParams();
    params.set<NonlinearVariableName>("variable") = variables[0];
    // add it to the warehouse
    _awh.addActionBlock(action);
  }

  // Setup our Diffusion Kernel on the "v" variable
  action = _action_factory.create("AddKernelAction", "Kernels/diff_v", action_params);

  moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");
  {
    InputParameters & params = moose_object_action->getObjectParams();
    params.set<NonlinearVariableName>("variable") = variables[1];
  }
  // add it to the warehouse
  _awh.addActionBlock(action);

  // Setup our Convection Kernel on the "u" variable coupled to the diffusion variable "v"
  action_params.set<std::string>("type") = "Convection";
  action = _action_factory.create("AddKernelAction", "Kernels/conv_u", action_params);
  moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");
  {
    std::vector<std::string> vel_vec_variable;
    InputParameters & params = moose_object_action->getObjectParams();
    params.set<NonlinearVariableName>("variable") = variables[0];
    vel_vec_variable.push_back(variables[1]);
    params.set<std::vector<std::string> >("some_variable") = vel_vec_variable;

    params.set<RealVectorValue>("velocity") = RealVectorValue(0, 0, 0);
  }
  // add it to the warehouse
  _awh.addActionBlock(action);
}
