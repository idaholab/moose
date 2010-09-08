#include "ConvectionDiffusionBlock.h"
#include "KernelFactory.h"
#include "Parser.h"

template<>
InputParameters validParams<ConvectionDiffusionBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  
  params.addParam<std::vector<std::string> >("variables", "The diffusion-convection and diffusion variables respectively");
  return params;
}

ConvectionDiffusionBlock::ConvectionDiffusionBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
ConvectionDiffusionBlock::execute() 
{
  std::vector<std::string> variables = getBlockParams().get<std::vector<std::string> >("variables");
  std::vector<std::string> vel_vec_variable;

  /**
   * We need to manually setup our Convection-Diffusion and Diffusion variables on our two
   * variables we are expecting from the input file.  Much of the syntax below is hidden by the
   * parser system but we have to set things up ourselves this time.
   */

  // Do some error checking
  mooseAssert(variables.size() == 2, "Expected 2 variables, received " + variables.size());

  // Setup our Diffusion Kernel on the "u" variable
  InputParameters conv_diff_params = KernelFactory::instance()->getValidParams("Diffusion");
  conv_diff_params.set<std::string>("variable") = variables[0];
  _moose_system.addKernel("Diffusion", "diff_u", conv_diff_params);

  // Setup our Convection Kernel on the "u" variable coupled to the diffusion variable "v"
  conv_diff_params.addCoupledVar("velocity_vector", "The Velocity Vector");
  vel_vec_variable.push_back(variables[1]);
  conv_diff_params.set<std::vector<std::string> >("velocity_vector") = vel_vec_variable;
  _moose_system.addKernel("Convection", "conv", conv_diff_params);

  // Setup out Diffusion Kernel on the "v" variable
  InputParameters conv_params = KernelFactory::instance()->getValidParams("Convection");
  conv_params.set<std::string>("variable") = variables[1];
  _moose_system.addKernel("Diffusion", "diff_v", conv_params);
}
