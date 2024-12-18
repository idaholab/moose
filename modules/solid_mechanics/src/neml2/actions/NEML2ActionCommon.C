//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

#ifdef NEML2_ENABLED
#include "neml2/base/Factory.h"
#include "neml2/misc/parser_utils.h"
#endif

registerMooseAction("SolidMechanicsApp", NEML2ActionCommon, "parse_neml2");

InputParameters
NEML2ActionCommon::commonParams()
{
  auto params = NEML2ModelExecutor::actionParams();

  MultiMooseEnum moose_types("MATERIAL VARIABLE POSTPROCESSOR");

  // Inputs
  params.addParam<MultiMooseEnum>(
      "moose_input_types",
      moose_types,
      NEML2Utils::docstring("Type of each MOOSE data to be used as NEML2 input variable"));
  params.addParam<std::vector<std::string>>(
      "moose_inputs",
      {},
      NEML2Utils::docstring("List of MOOSE data to be used as inputs of the material model."));
  params.addParam<std::vector<std::string>>(
      "neml2_inputs",
      {},
      NEML2Utils::docstring("List of NEML2 input variables corresponding to each MOOSE data."));

  // Model parameters
  params.addParam<MultiMooseEnum>(
      "moose_parameter_types",
      moose_types,
      NEML2Utils::docstring("Type of each MOOSE data to be used as NEML2 model parameter"));
  params.addParam<std::vector<std::string>>(
      "moose_parameters",
      {},
      NEML2Utils::docstring("List of MOOSE data to be used as parameters of the material model."));
  params.addParam<std::vector<std::string>>(
      "neml2_parameters",
      {},
      NEML2Utils::docstring("List of NEML2 model parameters corresponding to each MOOSE data."));

  // Outputs
  params.addParam<MultiMooseEnum>(
      "moose_output_types",
      moose_types,
      NEML2Utils::docstring("MOOSE types used to hold the NEML2 output variables"));
  params.addParam<std::vector<std::string>>(
      "moose_outputs",
      {},
      NEML2Utils::docstring("List of MOOSE data used to hold the output of the material model."));
  params.addParam<std::vector<std::string>>(
      "neml2_outputs",
      {},
      NEML2Utils::docstring("List of NEML2 output variables corresponding to each MOOSE data."));

  // Derivatives
  params.addParam<MultiMooseEnum>(
      "moose_derivative_types",
      moose_types,
      NEML2Utils::docstring("MOOSE types used to hold the NEML2 variable derivatives"));
  params.addParam<std::vector<std::string>>(
      "moose_derivatives",
      {},
      NEML2Utils::docstring(
          "List of MOOSE data used to hold the derivative of the material model."));
  params.addParam<std::vector<std::vector<std::string>>>(
      "neml2_derivatives",
      {},
      NEML2Utils::docstring("List of pairs of NEML2 variables to take derivatives (i.e., first in "
                            "the pair w.r.t. the second in the pair)."));

  // Parameter derivatives
  params.addParam<MultiMooseEnum>(
      "moose_parameter_derivative_types",
      moose_types,
      NEML2Utils::docstring("MOOSE types used to hold the NEML2 parameter derivatives"));
  params.addParam<std::vector<std::string>>(
      "moose_parameter_derivatives",
      {},
      NEML2Utils::docstring("List of MOOSE data used to hold the derivative of the material model "
                            "w.r.t. model parameters."));
  params.addParam<std::vector<std::vector<std::string>>>(
      "neml2_parameter_derivatives",
      {},
      NEML2Utils::docstring("List of pairs of NEML2 variables to take derivatives (i.e., first in "
                            "the pair w.r.t. the second in the pair)."));

  // Error checking, logging, etc
  params.addParam<std::vector<std::string>>(
      "skip_variables",
      {},
      NEML2Utils::docstring(
          "List of NEML2 variables to skip when setting up the model input. If an input variable "
          "is skipped, its value will stay zero. If a required input variable is not skipped, an "
          "error will be raised."));
  params.addParam<bool>("verbose",
                        true,
                        NEML2Utils::docstring("Whether to print additional information about the "
                                              "NEML2 model at the beginning of the simulation"));

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
  params.addClassDescription(NEML2Utils::docstring("Parse a NEML2 input file"));
  params.addRequiredParam<DataFileName>(
      "input",
      NEML2Utils::docstring("Path to the NEML2 input file containing the NEML2 model(s)."));
  params.addParam<std::vector<std::string>>(
      "cli_args",
      {},
      "Additional command line arguments to use when parsing the NEML2 input file.");
  return params;
}

NEML2ActionCommon::NEML2ActionCommon(const InputParameters & params)
  : Action(params),
    _fname(getParam<DataFileName>("input")),
    _cli_args(getParam<std::vector<std::string>>("cli_args"))
{
  NEML2Utils::assertNEML2Enabled();
}

void
NEML2ActionCommon::act()
{
#ifdef NEML2_ENABLED
  if (_current_task == "parse_neml2")
    neml2::load_input(std::string(_fname), neml2::utils::join(_cli_args, " "));
#endif
}
