//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2Action.h"
#include "NEML2ActionCommon.h"
#include "FEProblem.h"
#include "Factory.h"
#include "NEML2Utils.h"
#include "InputParameterWarehouse.h"

#ifdef NEML2_ENABLED
#include "neml2/misc/parser_utils.h"
#endif

registerMooseAction("SolidMechanicsApp", NEML2Action, "add_material");
registerMooseAction("SolidMechanicsApp", NEML2Action, "add_user_object");

#ifdef NEML2_ENABLED
// NEML2 variable type --> MOOSE type
const std::map<neml2::TensorType, std::string> tensor_type_map = {
    {neml2::TensorType::kScalar, "Real"},
    {neml2::TensorType::kSR2, "SymmetricRankTwoTensor"},
    {neml2::TensorType::kR2, "RankTwoTensor"},
    {neml2::TensorType::kSSR4, "SymmetricRankFourTensor"},
    {neml2::TensorType::kR4, "RankFourTensor"},
    {neml2::TensorType::kRot, "RealVectorValue"}};
// NEML2 (output, input) type --> NEML2 derivative type
const std::map<std::pair<neml2::TensorType, neml2::TensorType>, neml2::TensorType> deriv_type_map =
    {
        {{neml2::TensorType::kScalar, neml2::TensorType::kScalar}, neml2::TensorType::kScalar},
        {{neml2::TensorType::kSR2, neml2::TensorType::kSR2}, neml2::TensorType::kSSR4},
        {{neml2::TensorType::kSR2, neml2::TensorType::kScalar}, neml2::TensorType::kSR2},
        {{neml2::TensorType::kScalar, neml2::TensorType::kSR2}, neml2::TensorType::kSR2},
        {{neml2::TensorType::kR2, neml2::TensorType::kR2}, neml2::TensorType::kR4},
        {{neml2::TensorType::kR2, neml2::TensorType::kScalar}, neml2::TensorType::kR2},
        {{neml2::TensorType::kScalar, neml2::TensorType::kR2}, neml2::TensorType::kR2},
};
#endif

InputParameters
NEML2Action::validParams()
{
  InputParameters params = NEML2ActionCommon::commonParams();
  params.addClassDescription(NEML2Utils::docstring("Set up the NEML2 material model"));
  params.addParam<std::string>(
      "executor_name",
      NEML2Utils::docstring("Name of the NEML2ModelExecutor user object. The default name is "
                            "'neml2_<model-name>_<block-name>' where <model-name> is the NEML2 "
                            "model's name, and <block-name> is this action sub-block's name."));
  params.addParam<std::string>(
      "batch_index_generator_name",
      NEML2Utils::docstring(
          "Name of the NEML2BatchIndexGenerator user object. The default name is "
          "'neml2_index_<model-name>_<block-name>' where <model-name> is the NEML2 model's name, "
          "and <block-name> is this action sub-block's name."));
  params.addParam<std::vector<SubdomainName>>(
      "block",
      {},
      NEML2Utils::docstring("List of blocks (subdomains) where the material model is defined"));
  return params;
}

NEML2Action::NEML2Action(const InputParameters & params)
  : Action(params),
    _executor_name(isParamValid("executor_name")
                       ? getParam<std::string>("executor_name")
                       : "neml2_" + getParam<std::string>("model") + "_" + name()),
    _idx_generator_name(isParamValid("batch_index_generator_name")
                            ? getParam<std::string>("batch_index_generator_name")
                            : "neml2_index_" + getParam<std::string>("model") + "_" + name()),
    _block(getParam<std::vector<SubdomainName>>("block"))
{
  NEML2Utils::assertNEML2Enabled();

  // Apply parameters under the common area, i.e., under [NEML2]
  const auto & all_params = _app.getInputParameterWarehouse().getInputParameters();
  auto & sub_block_params = *(all_params.find(uniqueActionName())->second.get());
  const auto & common_action = getCommonAction();
  sub_block_params.applyParameters(common_action.parameters());

  // verbosity
  _verbose = getParam<bool>("verbose");

  // Set up optional output variable initialization
  auto init_vars = getParam<std::vector<MaterialPropertyName>>("initialize_outputs");
  auto init_vals = getParam<std::vector<MaterialPropertyName>>("initialize_output_values");
  if (init_vars.size() != init_vals.size())
    paramError("initialize_outputs",
               "initialize_outputs should have the same length as initialize_output_values");
  for (auto i : index_range(init_vars))
    _initialize_output_values[init_vars[i]] = init_vals[i];

  // Set up additional outputs for each requested material property
  auto outputs = getParam<std::vector<MaterialPropertyName>>("export_outputs");
  auto output_targets = getParam<std::vector<std::vector<OutputName>>>("export_output_targets");
  if (outputs.size() != output_targets.size())
    paramError("export_outputs",
               "export_outputs should have the same length as export_output_targets");
  for (auto i : index_range(outputs))
    _export_output_targets[outputs[i]] = output_targets[i];
}

const NEML2ActionCommon &
NEML2Action::getCommonAction() const
{
  auto common_block = _awh.getActions<NEML2ActionCommon>();
  mooseAssert(common_block.size() == 1, "There must exist one and only one common NEML2 action.");
  return *common_block[0];
}

#ifndef NEML2_ENABLED

void
NEML2Action::act()
{
}

#else

void
NEML2Action::act()
{
  if (_current_task == "add_user_object")
  {
    // Get the NEML2 model so that we can introspect variable tensor types
    auto & model = neml2::get_model(getParam<std::string>("model"));
    model.to(getParam<std::string>("device"));

    setupInputMappings(model);
    setupParameterMappings(model);
    setupOutputMappings(model);
    setupDerivativeMappings(model);
    setupParameterDerivativeMappings(model);

    if (_verbose)
      printSummary(model);

    // MOOSEToNEML2 input gatherers
    std::vector<UserObjectName> gatherers;
    for (const auto & input : _inputs)
    {
      if (input.moose.type == MOOSEIOType::MATERIAL)
      {
        auto obj_name = "__moose(" + input.moose.name + ")->neml2(" +
                        neml2::utils::stringify(input.neml2.name) + ")_" + name() + "__";
        if (!tensor_type_map.count(input.neml2.type))
          mooseError("NEML2 type ", input.neml2.type, " not yet mapped to MOOSE");
        auto obj_moose_type = tensor_type_map.at(input.neml2.type) + "MaterialProperty";
        if (input.neml2.name.is_old_force() || input.neml2.name.is_old_state())
          obj_moose_type = "Old" + obj_moose_type;
        auto obj_type = "MOOSE" + obj_moose_type + "ToNEML2";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<MaterialPropertyName>("from_moose") = input.moose.name;
        obj_params.set<std::string>("to_neml2") = neml2::utils::stringify(input.neml2.name);
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
        _problem->addUserObject(obj_type, obj_name, obj_params);
        gatherers.push_back(obj_name);
      }
      else if (input.moose.type == MOOSEIOType::VARIABLE)
      {
        auto obj_name = "__moose(" + input.moose.name + ")->neml2(" +
                        neml2::utils::stringify(input.neml2.name) + ")_" + name() + "__";
        std::string obj_moose_type = "Variable";
        if (input.neml2.name.is_old_force() || input.neml2.name.is_old_state())
          obj_moose_type = "Old" + obj_moose_type;
        auto obj_type = "MOOSE" + obj_moose_type + "ToNEML2";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<std::vector<VariableName>>("from_moose") = {input.moose.name};
        obj_params.set<std::string>("to_neml2") = neml2::utils::stringify(input.neml2.name);
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
        _problem->addUserObject(obj_type, obj_name, obj_params);
        gatherers.push_back(obj_name);
      }
      else if (input.moose.type == MOOSEIOType::POSTPROCESSOR)
      {
        auto obj_name = "__moose(" + input.moose.name + ")->neml2(" +
                        neml2::utils::stringify(input.neml2.name) + ")" + name() + "__";
        auto obj_moose_type = std::string("Postprocessor");
        if (input.neml2.name.is_old_force() || input.neml2.name.is_old_state())
          obj_moose_type = "Old" + obj_moose_type;
        auto obj_type = "MOOSE" + obj_moose_type + "ToNEML2";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<PostprocessorName>("from_moose") = input.moose.name;
        obj_params.set<std::string>("to_neml2") = neml2::utils::stringify(input.neml2.name);
        _problem->addUserObject(obj_type, obj_name, obj_params);
        gatherers.push_back(obj_name);
      }
      else
        paramError("moose_input_types",
                   "Unsupported type corresponding to the moose input ",
                   input.moose.name);
    }

    // MOOSEToNEML2 parameter gatherers
    std::vector<UserObjectName> param_gatherers;
    for (const auto & param : _params)
    {
      if (param.moose.type == MOOSEIOType::MATERIAL)
      {
        auto obj_name =
            "__moose(" + param.moose.name + ")->neml2(" + param.neml2.name + ")_" + name() + "__";
        if (!tensor_type_map.count(param.neml2.type))
          mooseError("NEML2 type ", param.neml2.type, " not yet mapped to MOOSE");
        auto obj_moose_type = tensor_type_map.at(param.neml2.type);
        auto obj_type = "MOOSE" + obj_moose_type + "MaterialPropertyToNEML2";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<MaterialPropertyName>("from_moose") = param.moose.name;
        obj_params.set<std::string>("to_neml2") = param.neml2.name;
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
        _problem->addUserObject(obj_type, obj_name, obj_params);
        param_gatherers.push_back(obj_name);
      }
      else if (param.moose.type == MOOSEIOType::VARIABLE)
      {
        auto obj_name =
            "__moose(" + param.moose.name + ")->neml2(" + param.neml2.name + ")_" + name() + "__";
        auto obj_type = "MOOSEVariableToNEML2";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<std::vector<VariableName>>("from_moose") = {param.moose.name};
        obj_params.set<std::string>("to_neml2") = neml2::utils::stringify(param.neml2.name);
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
        _problem->addUserObject(obj_type, obj_name, obj_params);
        param_gatherers.push_back(obj_name);
      }
      else if (param.moose.type == MOOSEIOType::POSTPROCESSOR)
      {
        auto obj_name =
            "__moose(" + param.moose.name + ")->neml2(" + param.neml2.name + ")" + name() + "__";
        auto obj_moose_type = std::string("Postprocessor");
        auto obj_type = "MOOSE" + obj_moose_type + "ToNEML2";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<PostprocessorName>("from_moose") = param.moose.name;
        obj_params.set<std::string>("to_neml2") = param.neml2.name;
        _problem->addUserObject(obj_type, obj_name, obj_params);
        param_gatherers.push_back(obj_name);
      }
      else
        paramError("moose_parameter_types",
                   "Unsupported type corresponding to the moose parameter ",
                   param.moose.name);
    }

    // The index generator UO
    {
      auto type = "NEML2BatchIndexGenerator";
      auto params = _factory.getValidParams(type);
      params.applyParameters(parameters());
      _problem->addUserObject(type, _idx_generator_name, params);
    }

    // The Executor UO
    {
      auto type = "NEML2ModelExecutor";
      auto params = _factory.getValidParams(type);
      params.applyParameters(parameters());
      params.set<UserObjectName>("batch_index_generator") = _idx_generator_name;
      params.set<std::vector<UserObjectName>>("gatherers") = gatherers;
      params.set<std::vector<UserObjectName>>("param_gatherers") = param_gatherers;
      _problem->addUserObject(type, _executor_name, params);
    }
  }

  if (_current_task == "add_material")
  {
    // NEML2ToMOOSE output retrievers
    for (const auto & output : _outputs)
    {
      if (output.moose.type == MOOSEIOType::MATERIAL)
      {
        auto obj_name = "__neml2(" + neml2::utils::stringify(output.neml2.name) + ")->moose(" +
                        output.moose.name + ")_" + name() + "__";
        if (!tensor_type_map.count(output.neml2.type))
          mooseError("NEML2 type ", output.neml2.type, " not yet mapped to MOOSE");
        auto obj_type = "NEML2ToMOOSE" + tensor_type_map.at(output.neml2.type) + "MaterialProperty";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<UserObjectName>("neml2_executor") = _executor_name;
        obj_params.set<MaterialPropertyName>("to_moose") = output.moose.name;
        obj_params.set<std::string>("from_neml2") = neml2::utils::stringify(output.neml2.name);
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
        if (_initialize_output_values.count(output.moose.name))
          obj_params.set<MaterialPropertyName>("moose_material_property_init") =
              _initialize_output_values[output.moose.name];
        if (_export_output_targets.count(output.moose.name))
          obj_params.set<std::vector<OutputName>>("outputs") =
              _export_output_targets[output.moose.name];
        _problem->addMaterial(obj_type, obj_name, obj_params);
      }
      else
        paramError("moose_output_types",
                   "Unsupported type corresponding to the moose output ",
                   output.moose.name);
    }

    // NEML2ToMOOSE derivative retrievers
    for (const auto & deriv : _derivs)
    {
      if (deriv.moose.type == MOOSEIOType::MATERIAL)
      {
        auto obj_name = "__neml2(d(" + neml2::utils::stringify(deriv.neml2.y.name) + ")/d(" +
                        neml2::utils::stringify(deriv.neml2.x.name) + "))->moose(" +
                        deriv.moose.name + ")_" + name() + "__";
        if (!deriv_type_map.count({deriv.neml2.y.type, deriv.neml2.x.type}))
          mooseError("NEML2 derivative type for d(",
                     deriv.neml2.y.type,
                     ")/d(",
                     deriv.neml2.x.type,
                     ") not yet mapped to MOOSE");
        auto deriv_type = deriv_type_map.at({deriv.neml2.y.type, deriv.neml2.x.type});
        if (!tensor_type_map.count(deriv_type))
          mooseError("NEML2 type ", deriv_type, " not yet mapped to MOOSE");
        auto obj_type = "NEML2ToMOOSE" + tensor_type_map.at(deriv_type) + "MaterialProperty";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<UserObjectName>("neml2_executor") = _executor_name;
        obj_params.set<MaterialPropertyName>("to_moose") = deriv.moose.name;
        obj_params.set<std::string>("from_neml2") = neml2::utils::stringify(deriv.neml2.y.name);
        obj_params.set<std::string>("neml2_input_derivative") =
            neml2::utils::stringify(deriv.neml2.x.name);
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
        if (_export_output_targets.count(deriv.moose.name))
          obj_params.set<std::vector<OutputName>>("outputs") =
              _export_output_targets[deriv.moose.name];
        _problem->addMaterial(obj_type, obj_name, obj_params);
      }
      else
        paramError("moose_derivative_types",
                   "Unsupported type corresponding to the moose derivative ",
                   deriv.moose.name);
    }

    // NEML2ToMOOSE parameter derivative retrievers
    for (const auto & param_deriv : _param_derivs)
    {
      if (param_deriv.moose.type == MOOSEIOType::MATERIAL)
      {
        auto obj_name = "__neml2(d(" + neml2::utils::stringify(param_deriv.neml2.y.name) + ")/d(" +
                        param_deriv.neml2.x.name + "))->moose(" + param_deriv.moose.name + ")_" +
                        name() + "__";
        if (!deriv_type_map.count({param_deriv.neml2.y.type, param_deriv.neml2.x.type}))
          mooseError("NEML2 derivative type for d(",
                     param_deriv.neml2.y.type,
                     ")/d(",
                     param_deriv.neml2.x.type,
                     ") not yet mapped to MOOSE");
        auto deriv_type = deriv_type_map.at({param_deriv.neml2.y.type, param_deriv.neml2.x.type});
        if (!tensor_type_map.count(deriv_type))
          mooseError("NEML2 type ", deriv_type, " not yet mapped to MOOSE");
        auto obj_type = "NEML2ToMOOSE" + tensor_type_map.at(deriv_type) + "MaterialProperty";
        auto obj_params = _factory.getValidParams(obj_type);
        obj_params.set<UserObjectName>("neml2_executor") = _executor_name;
        obj_params.set<MaterialPropertyName>("to_moose") = param_deriv.moose.name;
        obj_params.set<std::string>("from_neml2") =
            neml2::utils::stringify(param_deriv.neml2.y.name);
        obj_params.set<std::string>("neml2_parameter_derivative") = param_deriv.neml2.x.name;
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
        if (_export_output_targets.count(param_deriv.moose.name))
          obj_params.set<std::vector<OutputName>>("outputs") =
              _export_output_targets[param_deriv.moose.name];
        _problem->addMaterial(obj_type, obj_name, obj_params);
      }
      else
        paramError("moose_parameter_derivative_types",
                   "Unsupported type corresponding to the moose parameter derivative ",
                   param_deriv.moose.name);
    }
  }
}

void
NEML2Action::setupInputMappings(const neml2::Model & model)
{
  const auto [moose_input_types, moose_inputs, neml2_inputs] =
      getInputParameterMapping<MOOSEIOType, std::string, std::string>(
          "moose_input_types", "moose_inputs", "neml2_inputs");

  for (auto i : index_range(moose_inputs))
  {
    auto neml2_input = NEML2Utils::parseVariableName(neml2_inputs[i]);
    _inputs.push_back({
        {moose_inputs[i], moose_input_types[i]},
        {neml2_input, model.input_variable(neml2_input).type()},
    });
  }
}

void
NEML2Action::setupParameterMappings(const neml2::Model & model)
{
  const auto [moose_param_types, moose_params, neml2_params] =
      getInputParameterMapping<MOOSEIOType, std::string, std::string>(
          "moose_parameter_types", "moose_parameters", "neml2_parameters");

  for (auto i : index_range(moose_params))
    _params.push_back({{moose_params[i], moose_param_types[i]},
                       {neml2_params[i], model.get_parameter(neml2_params[i]).type()}});
}

void
NEML2Action::setupOutputMappings(const neml2::Model & model)
{
  const auto [moose_output_types, moose_outputs, neml2_outputs] =
      getInputParameterMapping<MOOSEIOType, std::string, std::string>(
          "moose_output_types", "moose_outputs", "neml2_outputs");

  for (auto i : index_range(moose_outputs))
  {
    auto neml2_output = NEML2Utils::parseVariableName(neml2_outputs[i]);
    _outputs.push_back({
        {moose_outputs[i], moose_output_types[i]},
        {neml2_output, model.output_variable(neml2_output).type()},
    });
  }
}

void
NEML2Action::setupDerivativeMappings(const neml2::Model & model)
{
  const auto [moose_deriv_types, moose_derivs, neml2_derivs] =
      getInputParameterMapping<MOOSEIOType, std::string, std::vector<std::string>>(
          "moose_derivative_types", "moose_derivatives", "neml2_derivatives");

  for (auto i : index_range(moose_derivs))
  {
    if (neml2_derivs[i].size() != 2)
      paramError("neml2_derivatives", "The length of each pair in neml2_derivatives must be 2.");

    auto neml2_y = NEML2Utils::parseVariableName(neml2_derivs[i][0]);
    auto neml2_x = NEML2Utils::parseVariableName(neml2_derivs[i][1]);
    _derivs.push_back({
        {moose_derivs[i], moose_deriv_types[i]},
        {{neml2_y, model.output_variable(neml2_y).type()},
         {neml2_x, model.input_variable(neml2_x).type()}},
    });
  }
}

void
NEML2Action::setupParameterDerivativeMappings(const neml2::Model & model)
{
  const auto [moose_param_deriv_types, moose_param_derivs, neml2_param_derivs] =
      getInputParameterMapping<MOOSEIOType, std::string, std::vector<std::string>>(
          "moose_parameter_derivative_types",
          "moose_parameter_derivatives",
          "neml2_parameter_derivatives");

  for (auto i : index_range(moose_param_derivs))
  {
    if (neml2_param_derivs[i].size() != 2)
      paramError("neml2_parameter_derivatives",
                 "The length of each pair in neml2_parameter_derivatives must be 2.");

    auto neml2_y = NEML2Utils::parseVariableName(neml2_param_derivs[i][0]);
    auto neml2_x = neml2_param_derivs[i][1];
    _param_derivs.push_back({
        {moose_param_derivs[i], moose_param_deriv_types[i]},
        {{neml2_y, model.output_variable(neml2_y).type()},
         {neml2_x, model.get_parameter(neml2_x).type()}},
    });
  }
}

void
NEML2Action::printSummary(const neml2::Model & model) const
{
  // Save formatting of the output stream so that we can restore it later
  const auto flags = _console.flags();

  // Default width for the summary
  const int width = 79;

  _console << std::endl;
  _console << COLOR_CYAN << std::setw(width) << std::setfill('*') << std::left
           << "NEML2 MATERIAL MODEL SUMMARY BEGIN " << std::setfill(' ') << COLOR_DEFAULT
           << std::endl;

  // Metadata
  _console << "NEML2 input file location: " << getCommonAction().fname() << std::endl;
  _console << "NEML2 action path:         " << parameters().blockFullpath() << std::endl;

  // List inputs, outputs, and parameters of the model
  _console << COLOR_CYAN << std::setw(width) << std::setfill('-') << std::left
           << "Material model structure " << std::setfill(' ') << COLOR_DEFAULT << std::endl;
  _console << model;

  // List transfer between MOOSE and NEML2
  _console << COLOR_CYAN << std::setw(width) << std::setfill('-') << std::left
           << "Transfer between MOOSE and NEML2 " << std::setfill(' ') << COLOR_DEFAULT
           << std::endl;

  // Figure out the longest name length so that we could align the arrows
  const auto max_moose_name_length = getLongestMOOSEName();

  // List input transfer, MOOSE -> NEML2
  for (const auto & input : _inputs)
  {
    _console << std::setw(max_moose_name_length) << std::right
             << (input.neml2.name.is_old_force() || input.neml2.name.is_old_state()
                     ? ("(old) " + input.moose.name)
                     : input.moose.name)
             << " --> " << input.neml2.name << std::endl;
  }

  // List parameter transfer, MOOSE -> NEML2
  for (const auto & param : _params)
    _console << std::setw(max_moose_name_length) << std::right << param.moose.name << " --> "
             << param.neml2.name << std::endl;

  // List output transfer, NEML2 -> MOOSE
  for (const auto & output : _outputs)
    _console << std::setw(max_moose_name_length) << std::right << output.moose.name << " <-- "
             << output.neml2.name << std::endl;

  // List derivative transfer, NEML2 -> MOOSE
  for (const auto & deriv : _derivs)
    _console << std::setw(max_moose_name_length) << std::right << deriv.moose.name << " <-- d("
             << deriv.neml2.y.name << ")/d(" << deriv.neml2.x.name << ")" << std::endl;

  // List parameter derivative transfer, NEML2 -> MOOSE
  for (const auto & param_deriv : _param_derivs)
    _console << std::setw(max_moose_name_length) << std::right << param_deriv.moose.name
             << " <-- d(" << param_deriv.neml2.y.name << ")/d(" << param_deriv.neml2.x.name << ")"
             << std::endl;

  _console << COLOR_CYAN << std::setw(width) << std::setfill('*') << std::left
           << "NEML2 MATERIAL MODEL SUMMARY END " << std::setfill(' ') << COLOR_DEFAULT << std::endl
           << std::endl;

  // Restore the formatting of the output stream
  _console.flags(flags);
}

std::size_t
NEML2Action::getLongestMOOSEName() const
{
  std::size_t max_moose_name_length = 0;
  for (const auto & input : _inputs)
    max_moose_name_length =
        std::max(max_moose_name_length,
                 input.neml2.name.is_old_force() || input.neml2.name.is_old_state()
                     ? input.moose.name.size() + 6
                     : input.moose.name.size()); // 6 is the length of "(old) "
  for (const auto & param : _params)
    max_moose_name_length = std::max(max_moose_name_length, param.moose.name.size());
  for (const auto & output : _outputs)
    max_moose_name_length = std::max(max_moose_name_length, output.moose.name.size());
  for (const auto & deriv : _derivs)
    max_moose_name_length = std::max(max_moose_name_length, deriv.moose.name.size());
  for (const auto & param_deriv : _param_derivs)
    max_moose_name_length = std::max(max_moose_name_length, param_deriv.moose.name.size());
  return max_moose_name_length;
}
#endif // NEML2_ENABLED
