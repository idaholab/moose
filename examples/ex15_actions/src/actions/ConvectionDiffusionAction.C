//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectionDiffusionAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

registerMooseAction("ExampleApp", ConvectionDiffusionAction, "add_kernel");

InputParameters
ConvectionDiffusionAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "variables", "The names of the convection and diffusion variables in the simulation");

  return params;
}

ConvectionDiffusionAction::ConvectionDiffusionAction(const InputParameters & params)
  : Action(params)
{
}

void
ConvectionDiffusionAction::act()
{
  std::vector<NonlinearVariableName> variables =
      getParam<std::vector<NonlinearVariableName>>("variables");
  std::vector<VariableName> vel_vec_variable;

  /**
   * We need to manually setup our Convection-Diffusion and Diffusion variables on our two
   * variables we are expecting from the input file.  Much of the syntax below is hidden by the
   * parser system but we have to set things up ourselves this time.
   */

  // Do some error checking
  mooseAssert(variables.size() == 2, "Expected 2 variables, received " << variables.size());

  // Setup our Diffusion Kernel on the "u" variable
  {
    InputParameters params = _factory.getValidParams("Diffusion");
    params.set<NonlinearVariableName>("variable") = variables[0];
    _problem->addKernel("Diffusion", "diff_u", params);
  }

  // Setup our Convection Kernel on the "u" variable coupled to the diffusion variable "v"
  {
    InputParameters params = _factory.getValidParams("ExampleConvection");
    params.set<NonlinearVariableName>("variable") = variables[0];
    //    params.addCoupledVar("some_variable", "The gradient of this var");
    vel_vec_variable.push_back(variables[1]);
    params.set<std::vector<VariableName>>("some_variable") = vel_vec_variable;
    _problem->addKernel("ExampleConvection", "conv", params);
  }

  // Setup out Diffusion Kernel on the "v" variable
  {
    InputParameters params = _factory.getValidParams("Diffusion");
    params.set<NonlinearVariableName>("variable") = variables[1];
    _problem->addKernel("Diffusion", "diff_v", params);
  }
}
