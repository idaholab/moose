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
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<ConvDiffMetaAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<std::string> >("variables", "The names of the convection and diffusion variables in the simulation");

  return params;
}

ConvDiffMetaAction::ConvDiffMetaAction(const std::string & name, InputParameters params)
  :Action(name, params)
{
}

void
ConvDiffMetaAction::act()
{
  Action *action;
  MooseObjectAction *moose_object_action;
  
  std::vector<std::string> variables = getParam<std::vector<std::string> > ("variables");

  std::cerr << "Acting on " << getParam<std::string>("built_by_action") << "\n\n";
  
  /**
   * We need to manually setup our Convection-Diffusion and Diffusion variables on our two
   * variables we are expecting from the input file.  Much of the syntax below is hidden by the
   * parser system but we have to set things up ourselves this time.
   */

  // Do some error checking
  mooseAssert(variables.size() == 2, "Expected 2 variables, received " + variables.size());

  //*******************************************//
  //**************** Variables ****************//
  //*******************************************//  
  InputParameters action_params = ActionFactory::instance()->getValidParams("Variables/*");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");

  // Creat and Add First Variable Action
  action = ActionFactory::instance()->create("Variables/" + variables[0], action_params);
  Moose::action_warehouse.addActionBlock(action);
  
  // Create and Add Second Variable Action
  action = ActionFactory::instance()->create("Variables/" + variables[1], action_params);
  Moose::action_warehouse.addActionBlock(action);

  //*******************************************//
  //**************** Kernels ******************//
  //*******************************************//
  action_params = ActionFactory::instance()->getValidParams("Kernels/*");
  action_params.set<Parser *>("parser_handle") = getParam<Parser *>("parser_handle");

  // Setup our Diffusion Kernel on the "u" variable
  action_params.set<std::string>("type") = "Diffusion";
  action = ActionFactory::instance()->create("Kernels/diff_u", action_params);
  moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");
  {
    InputParameters & params = moose_object_action->getMooseObjectParams();
    params.set<std::string>("variable") = variables[0];
  // add it to the warehouse
  Moose::action_warehouse.addActionBlock(action);
  }

  // Setup our Diffusion Kernel on the "v" variable
  action = ActionFactory::instance()->create("Kernels/diff_v", action_params);
  moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");
  {
    InputParameters & params = moose_object_action->getMooseObjectParams();
    params.set<std::string>("variable") = variables[1];
  }
  // add it to the warehouse
  Moose::action_warehouse.addActionBlock(action);

  // Setup our Convection Kernel on the "u" variable coupled to the diffusion variable "v"
  action_params.set<std::string>("type") = "Convection";
  action = ActionFactory::instance()->create("Kernels/conv_u", action_params);
  moose_object_action = dynamic_cast<MooseObjectAction *>(action);
  mooseAssert (moose_object_action, "Dynamic Cast failed");
  {
    std::vector<std::string> vel_vec_variable;
    InputParameters & params = moose_object_action->getMooseObjectParams();
    params.set<std::string>("variable") = variables[0];
    vel_vec_variable.push_back(variables[1]);
    params.set<std::vector<std::string> >("some_variable") = vel_vec_variable;

    params.set<Real>("x") = 0;
    params.set<Real>("y") = 0;
  }
  // add it to the warehouse
  Moose::action_warehouse.addActionBlock(action);

}

