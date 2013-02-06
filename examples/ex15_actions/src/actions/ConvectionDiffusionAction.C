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

#include "ConvectionDiffusionAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<ConvectionDiffusionAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName> >("variables", "The names of the convection and diffusion variables in the simulation");

  return params;
}

ConvectionDiffusionAction::ConvectionDiffusionAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
ConvectionDiffusionAction::act()
{
  std::vector<NonlinearVariableName> variables = getParam<std::vector<NonlinearVariableName> > ("variables");
  std::vector<NonlinearVariableName> vel_vec_variable;

  /**
   * We need to manually setup our Convection-Diffusion and Diffusion variables on our two
   * variables we are expecting from the input file.  Much of the syntax below is hidden by the
   * parser system but we have to set things up ourselves this time.
   */

  // Do some error checking
  mooseAssert(variables.size() == 2, "Expected 2 variables, received " + variables.size());

  // Setup our Diffusion Kernel on the "u" variable
  InputParameters conv_diff_params = _factory.getValidParams("Diffusion");
  conv_diff_params.set<NonlinearVariableName>("variable") = variables[0];
  _problem->addKernel("Diffusion", "diff_u", conv_diff_params);

  // Setup our Convection Kernel on the "u" variable coupled to the diffusion variable "v"
  conv_diff_params.addCoupledVar("some_variable", "The gradient of this var");
  vel_vec_variable.push_back(variables[1]);
  conv_diff_params.set<std::vector<NonlinearVariableName> >("some_variable") = vel_vec_variable;
  _problem->addKernel("Convection", "conv", conv_diff_params);

  // Setup out Diffusion Kernel on the "v" variable
  InputParameters conv_params = _factory.getValidParams("Convection");
  conv_params.set<NonlinearVariableName>("variable") = variables[1];
  _problem->addKernel("Diffusion", "diff_v", conv_params);
}

