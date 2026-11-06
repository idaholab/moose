//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2ActionCommon.h"
#include "NEML2Action.h"
#include "NEML2Utils.h"
#include "NEML2ModelExecutor.h"

registerMooseAction("MooseApp", NEML2ActionCommon, "parse_neml2");

InputParameters
NEML2ActionCommon::commonParams()
{
  auto params = NEML2ModelInterface<Action>::validParams();
  params += NEML2ModelExecutor::actionParams();

  MultiMooseEnum moose_types("TIME SCALAR FUNCTION VARIABLE MATERIAL");

  // Inputs
  params.addParam<MultiMooseEnum>(
      "input_types", moose_types, "Type of each MOOSE data to be used as NEML2 input variable");
  params.addParam<std::vector<std::string>>(
      "inputs", {}, "List of NEML2 input variables corresponding to each MOOSE data.");
  params.addParam<std::vector<std::string>>(
      "input_kernels",
      {},
      "NEML2 kernels defined in MOOSE that provides input data. The object name must match the "
      "input variable name.");

  // Model parameters
  params.addParam<MultiMooseEnum>("parameter_types",
                                  moose_types,
                                  "Type of each MOOSE data to be used as NEML2 model parameter");
  params.addParam<std::vector<std::string>>(
      "parameters", {}, "List of NEML2 model parameters corresponding to each MOOSE data.");

  // Output
  params.addParam<bool>("auto_output",
                        true,
                        "Whether to automatically retrieve all NEML2 output variables as MOOSE "
                        "material properties.");

  // Derivatives
  params.addParam<std::vector<std::vector<std::string>>>(
      "derivatives",
      {},
      "List of pairs of NEML2 variables to take derivatives (i.e., first in the pair w.r.t. the "
      "second in the pair).");

  // Parameter derivatives
  params.addParam<std::vector<std::vector<std::string>>>(
      "parameter_derivatives",
      {},
      "List of pairs of NEML2 variables to take derivatives (i.e., first in the pair w.r.t. the "
      "second in the pair).");

  // Parameter derivative vector-Jacobian products
  params.addParam<std::string>(
      "parameter_vjp_variable",
      "NEML2 output variable whose parameter derivatives are contracted with the cotangent "
      "gathered from parameter_vjp_cotangent. Required when parameter_vjp_parameters is not "
      "empty.");
  params.addParam<MaterialPropertyName>(
      "parameter_vjp_cotangent",
      "MOOSE material property supplying the cotangent of parameter_vjp_variable, e.g. the adjoint "
      "strain. Note that the gatherer created for this property writes under the name of the NEML2 "
      "OUTPUT VARIABLE the cotangent is paired with (parameter_vjp_variable), not under the name "
      "of a NEML2 input variable. Required when parameter_vjp_parameters is not empty.");
  params.addParam<std::vector<std::string>>(
      "parameter_vjp_parameters",
      {},
      "List of NEML2 model parameters whose derivative of parameter_vjp_variable is contracted "
      "with the gathered cotangent. Each contraction is retrieved as a Real MOOSE material "
      "property named 'vjp_<parameter_vjp_variable>_<parameter>'. Every listed model parameter "
      "must be gathered from a MOOSE function so that it is batched per quadrature point.");

  // Execution scheduling of the objects this action creates
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR, EXEC_TIMESTEP_END};
  params.addParam<ExecFlagEnum>(
      "execute_on",
      execute_options,
      "Execution flags for the NEML2 model executor and for the MOOSEToNEML2 gatherers it depends "
      "on. Leave this unset unless the NEML2 model must also be evaluated outside the forward "
      "solve, for example on an adjoint execution flag when computing parameter derivative "
      "vector-Jacobian products.");
  params.addParam<int>(
      "execution_order_group",
      0,
      "Execution order group of the user objects created by this action. Execution order groups "
      "are executed in increasing order, so a negative number runs the NEML2 executor ahead of "
      "objects that consume its material properties in the default (0) group.");

  // Error checking, logging, etc
  params.addParam<std::vector<std::string>>(
      "skip_input_variables",
      {},
      "List of NEML2 variables to skip when setting up the model input. If an input variable is "
      "skipped, its value will stay zero. If a required input variable is skipped, an error "
      "will be raised.");
  params.addParam<bool>("verbose",
                        true,
                        "Whether to print additional information about the NEML2 model at the "
                        "beginning of the simulation");

  params.addParam<std::vector<MaterialPropertyName>>(
      "initialize_outputs",
      {},
      "List of MOOSE material properties to be initialized. Each these properties must correspond "
      "to a stateful NEML2 variable (which appears on both the input old state sub-axis and the "
      "output state sub-axis). These MOOSE material properties will be initialized with the values "
      "of properties specified in the initialize_output_values list.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "initialize_output_values",
      {},
      "List of MOOSE material properties whose initial values (evaluated at the 0th time step) "
      "will be used to initialize stateful properties. See the description of initialize_outputs "
      "for more details.");

  params.addParam<std::vector<MaterialPropertyName>>(
      "export_outputs",
      {},
      "List of MOOSE material properties to export which correspond to NEML2 output variables or "
      "output derivatives. Each material property's export targets can be specified by "
      "export_output_targets. The default export target is 'none'.");
  params.addParam<std::vector<std::vector<OutputName>>>(
      "export_output_targets",
      {},
      "The export targets corresponding to each MOOSE material property specified in "
      "export_outputs.");

  return params;
}

InputParameters
NEML2ActionCommon::validParams()
{
  auto params = NEML2ActionCommon::commonParams();
  params.addClassDescription("Parse a NEML2 input file");
  return params;
}

NEML2ActionCommon::NEML2ActionCommon(const InputParameters & params) : Action(params)
{
  NEML2Utils::assertNEML2Enabled();
}
