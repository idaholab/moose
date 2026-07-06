//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "neml2/neml2.h"
#include "neml2/base/Settings.h"
#include "neml2/models/VariableBase.h"
#endif

registerMooseAction("MooseApp", NEML2Action, "parse_neml2");
registerMooseAction("MooseApp", NEML2Action, "add_material");
registerMooseAction("MooseApp", NEML2Action, "add_user_object");

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
  params.addClassDescription("Set up the NEML2 material model");
  params.addParam<std::string>("executor_name",
                               "Name of the NEML2ModelExecutor user object. The default name is "
                               "'neml2_<model-name>_<block-name>' where <model-name> is the NEML2 "
                               "model's name, and <block-name> is this action sub-block's name.");
  params.addParam<std::string>(
      "batch_index_generator_name",
      "Name of the NEML2BatchIndexGenerator user object. The default name is "
      "'neml2_index_<model-name>_<block-name>' where <model-name> is the NEML2 model's name, and "
      "<block-name> is this action sub-block's name.");
  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "List of blocks (subdomains) where the material model is defined");
  params.addParam<std::vector<BoundaryName>>(
      "interface", {}, "List of interfaces where the material model is defined");
  params.addParam<bool>("interface_only",
                        false,
                        "If true, only create the interface material on the boundaries listed in "
                        "'interface' and skip "
                        "creating any volume material. Requires 'interface' to be set.");
  // Block materials need no hint: the same material is reinit'd as BLOCK/FACE/NEIGHBOR data and the
  // gatherer reads the store for where it is. A true InterfaceMaterial instead lives in the
  // separate INTERFACE_MATERIAL_DATA store, which the native block/face/neighbor selection never
  // picks. Since this action builds its gatherers on add_user_object -- before materials exist --
  // it cannot query the warehouse to tell the two apart from a property name alone, so the
  // interface-material inputs are named here. Per-input so one action can mix both (e.g. 'jump'
  // from an InterfaceMaterial and 'stiffness' from a block material).
  params.addParam<std::vector<MaterialPropertyName>>(
      "interface_material_inputs",
      {},
      "MATERIAL inputs supplied by a true InterfaceMaterial (read from interface material data "
      "instead of the volume/side material data). Ignored for non-MATERIAL inputs.");
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
    _block(getParam<std::vector<SubdomainName>>("block")),
    _interface(getParam<std::vector<BoundaryName>>("interface")),
    _interface_only(getParam<bool>("interface_only")),
    _interface_material_inputs(
        getParam<std::vector<MaterialPropertyName>>("interface_material_inputs")),
    _skip_input_variables(getParam<std::vector<std::string>>("skip_input_variables"))
{
  NEML2Utils::assertNEML2Enabled();

  // Apply parameters under the common area, i.e., under [NEML2]
  const auto & all_params = _app.getInputParameterWarehouse().getInputParameters();
  auto & sub_block_params = *(all_params.find(uniqueActionName())->second.get());
  const auto & common_action = getCommonAction();
  sub_block_params.applyParameters(common_action.parameters());

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

#ifdef NEML2_ENABLED
  // File name and CLI args
  _fname = getParam<DataFileName>("input");
  _cli_args = getParam<std::vector<std::string>>("cli_args");

  // Load input file
  auto factory = neml2::load_input(std::string(_fname), neml2::utils::join(_cli_args, " "));
  _model = NEML2Utils::getModel(*factory, getParam<std::string>("model"));
#endif
}

const NEML2ActionCommon &
NEML2Action::getCommonAction() const
{
  auto common_block = _awh.getActions<NEML2ActionCommon>();
  mooseAssert(common_block.size() == 1, "There must exist one and only one common NEML2 action.");
  return *common_block[0];
}

void
NEML2Action::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  // The NEML2 gatherers and batch index generator are DomainUserObjects, which require one layer of
  // neighbor ghosting (declared in DomainUserObject::validParams) so that neighbor data can be
  // reinitialized on internal sides in parallel. These user objects are created during the
  // add_user_object task, which is too late for their relationship managers to attach, so declare
  // the ghosting here on the action's behalf.
  auto params = _factory.getValidParams("NEML2BatchIndexGenerator");
  Action::addRelationshipManagers(input_rm_type, params);
}

#ifndef NEML2_ENABLED

void
NEML2Action::act()
{
}

#else

static std::string
obscureObjectName(const std::string & name,
                  const std::string & prefix,
                  const std::string & suffix,
                  const std::string & block)
{
  return "__" + prefix + "_" + name + "_" + suffix + "_" + block + "__";
}

void
NEML2Action::act()
{
  if (_current_task == "parse_neml2")
  {
    if (_app.parameters().have_parameter<bool>("parse_neml2_only"))
      if (!_app.parameters().get<bool>("parse_neml2_only"))
        return;
    printSummary();
  }

  // Look up the MOOSE tensor type string for a NEML2 tensor type, or error
  auto mooseType = [this](neml2::TensorType type) -> const std::string &
  {
    auto it = tensor_type_map.find(type);
    if (it == tensor_type_map.end())
      mooseError("NEML2 type ", type, " not yet mapped to MOOSE");
    return it->second;
  };

  // Whether this action is block/interface restricted
  const bool is_blk = !_block.empty();
  const bool is_interface = !_interface.empty();

  if (_interface_only)
  {
    if (!is_interface)
      paramError("interface_only",
                 "'interface_only' is true, but 'interface' is unset. When 'interface_only' is "
                 "true, 'interface' must list the boundaries on which to create the material.");
  }

  if (_current_task == "add_user_object")
  {
    setupInputMappings(*_model);
    setupParameterMappings(*_model);
    setupOutputMappings(*_model);
    setupDerivativeMappings(*_model);
    setupParameterDerivativeMappings(*_model);

    printSummary();

    // Create and register a MOOSEToNEML2 gatherer user object, returning its name
    auto addGatherer = [&](const std::string & moose_name,
                           const std::string & neml2_name,
                           NEML2Utils::MOOSEIOType moose_type,
                           neml2::TensorType neml2_type,
                           const std::string & suffix,
                           const std::string & type_prefix = "")
    {
      auto obj_name = obscureObjectName(moose_name, "moose_to_neml2", suffix, name());
      auto obj_type = "MOOSE" + type_prefix + mooseType(neml2_type) + "ToNEML2";
      auto obj_params = _factory.getValidParams(obj_type);
      obj_params.set<std::string>("from_moose") = moose_name;
      obj_params.set<std::string>("to_neml2") = neml2_name;
      obj_params.set<MooseEnum>("quantity_type").assign(static_cast<int>(moose_type));
      if (is_blk)
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
      if (is_interface)
        obj_params.set<std::vector<BoundaryName>>("interface_boundaries") = _interface;
      if (_interface_only)
        obj_params.set<bool>("interface_only") = true;
      // Only a MATERIAL input listed in 'interface_material_inputs' reads from interface material
      // data; every other input keeps the default volume/side (block/face-neighbor) source.
      if (moose_type == NEML2Utils::MOOSEIOType::MATERIAL &&
          std::find(_interface_material_inputs.begin(),
                    _interface_material_inputs.end(),
                    moose_name) != _interface_material_inputs.end())
        obj_params.set<bool>("from_interface_material") = true;
      _problem->addUserObject(obj_type, obj_name, obj_params);
      return obj_name;
    };

    // MOOSEToNEML2 input gatherers
    std::vector<UserObjectName> gatherers;
    const auto sep = _model->settings().history_separator();
    for (const auto & input : _inputs)
      gatherers.push_back(addGatherer(input.name,
                                      neml2::history_name(input.name, input.history_order, sep),
                                      input.moose_type,
                                      input.neml2_type,
                                      std::to_string(input.history_order),
                                      input.history_order == 1 ? "Old" : ""));

    // Additional NEML2Kernels that provide input data
    for (const auto & kernel_name : getParam<std::vector<std::string>>("input_kernels"))
    {
      if (!_model->input_variables().count(kernel_name))
        paramError("input_kernels",
                   "The NEML2 kernel ",
                   kernel_name,
                   " name does not match any NEML2 input variable.");
      gatherers.push_back(kernel_name);
    }

    // MOOSEToNEML2 parameter gatherers
    std::vector<UserObjectName> param_gatherers;
    for (const auto & param : _params)
      param_gatherers.push_back(
          addGatherer(param.name, param.name, param.moose_type, param.neml2_type, ""));

    // The index generator UO
    {
      auto type = "NEML2BatchIndexGenerator";
      auto params = _factory.getValidParams(type);
      params.applyParameters(parameters());
      if (is_blk)
        params.set<std::vector<SubdomainName>>("block") = _block;
      if (is_interface)
        params.set<std::vector<BoundaryName>>("interface_boundaries") = _interface;
      params.set<bool>("interface_only") = _interface_only;
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
    // Look up the NEML2 derivative tensor type for (y, x) variables, or error
    auto derivTensorType =
        [this](neml2::TensorType y_type, neml2::TensorType x_type, const std::string & deriv_name)
    {
      auto it = deriv_type_map.find({y_type, x_type});
      if (it == deriv_type_map.end())
        mooseError("NEML2 derivative type for ", deriv_name, " not yet mapped to MOOSE");
      return it->second;
    };

    // Create and register a NEML2ToMOOSE material property retriever; `extra` lets
    // each caller add the bits that are unique to outputs vs. (parameter) derivatives
    auto addRetriever = [&](const std::string & moose_name,
                            const std::string & neml2_var,
                            neml2::TensorType tensor_type,
                            auto && extra)
    {
      auto obj_name = obscureObjectName(moose_name, "neml2_to_moose", "", name());
      auto obj_type = "NEML2ToMOOSE" + mooseType(tensor_type) + "MaterialProperty";
      auto obj_params = _factory.getValidParams(obj_type);
      obj_params.set<UserObjectName>("neml2_executor") = _executor_name;
      obj_params.set<MaterialPropertyName>("to_moose") = moose_name;
      obj_params.set<std::string>("from_neml2") = neml2_var;
      // In interface_only mode the model is only evaluated at interface QPs (no volume batch
      // indices), so the retriever must be boundary-restricted to look up side batch indices.
      if (_interface_only)
        obj_params.set<std::vector<BoundaryName>>("boundary") = _interface;
      else
        obj_params.set<std::vector<SubdomainName>>("block") = _block;
      if (_export_output_targets.count(moose_name))
        obj_params.set<std::vector<OutputName>>("outputs") = _export_output_targets[moose_name];
      extra(obj_params);
      _problem->addMaterial(obj_type, obj_name, obj_params);
    };

    // NEML2ToMOOSE output retrievers
    for (const auto & output : _outputs)
    {
      if (output.moose_type != NEML2Utils::MOOSEIOType::MATERIAL)
        paramError("moose_output_types",
                   "Unsupported type corresponding to the moose output ",
                   output.name);
      addRetriever(output.name,
                   output.name,
                   output.neml2_type,
                   [&](InputParameters & p)
                   {
                     if (_initialize_output_values.count(output.name))
                       p.set<MaterialPropertyName>("moose_material_property_init") =
                           _initialize_output_values[output.name];
                   });
    }

    // NEML2ToMOOSE derivative retrievers
    for (const auto & deriv : _derivs)
    {
      auto type = derivTensorType(_model->output_variable(deriv.y).type(),
                                  _model->input_variable(deriv.x).type(),
                                  deriv.name);
      addRetriever(deriv.name,
                   deriv.y,
                   type,
                   [&](InputParameters & p)
                   { p.set<std::string>("neml2_input_derivative") = deriv.x; });
    }

    // NEML2ToMOOSE parameter derivative retrievers
    for (const auto & param_deriv : _param_derivs)
    {
      auto type = derivTensorType(_model->output_variable(param_deriv.y).type(),
                                  _model->get_parameter(param_deriv.x).type(),
                                  param_deriv.name);
      addRetriever(param_deriv.name,
                   param_deriv.y,
                   type,
                   [&](InputParameters & p)
                   { p.set<std::string>("neml2_parameter_derivative") = param_deriv.x; });
    }
  }
}

NEML2Utils::MOOSEIOType
NEML2Action::inferMOOSEIOType(const neml2::VariableName & name,
                              const neml2::TensorType & type) const
{
  // neml2::kScalar can only come from scalar variable, function, or variable
  if (type == neml2::TensorType::kScalar)
  {
    bool is_time = _problem->isTransient() && (name == "t" || name == "time");
    bool has_scalar = _problem->hasScalarVariable(name);
    bool has_func = _problem->hasFunction(name);
    bool has_var = _problem->hasVariable(name);
    if (int(is_time) + int(has_scalar) + int(has_func) + int(has_var) > 1)
      mooseError("Trying to infer MOOSE data type for NEML2 variable ",
                 name,
                 ". The name matches multiple types (",
                 (is_time ? "time " : ""),
                 (has_scalar ? "scalar variable " : ""),
                 (has_func ? "function " : ""),
                 (has_var ? "variable " : ""),
                 "). To avoid ambiguity, please explicitly specify the type in the NEML2 action.");
    if (is_time)
      return NEML2Utils::MOOSEIOType::TIME;
    if (has_scalar)
      return NEML2Utils::MOOSEIOType::SCALAR;
    if (has_func)
      return NEML2Utils::MOOSEIOType::FUNCTION;
    if (has_var)
      return NEML2Utils::MOOSEIOType::VARIABLE;
    // if neither function nor variable exists, let's assume it's a material property
    // note that we can't explicitly check if a material property with the given name exists,
    // because materials are added _after_ user objects (see Moose.C)
    if (!has_func && !has_var)
      return NEML2Utils::MOOSEIOType::MATERIAL;
  }

  // non-scalar can only come from material properties
  return NEML2Utils::MOOSEIOType::MATERIAL;
}

void
NEML2Action::setupInputMappings(const neml2::Model & model)
{
  const auto & kernels = getParam<std::vector<std::string>>("input_kernels");

  // Default mapping
  for (const auto & [vname, var] : model.input_variables())
  {
    // user requested to skip
    if (std::find(_skip_input_variables.begin(), _skip_input_variables.end(), var->base_name()) !=
        _skip_input_variables.end())
      continue;

    // skip if the input is directly provided by a custom MOOSEToNEML2 object
    bool gathered_by_kernel = false;
    for (const auto & kernel_name : kernels)
      if (vname == kernel_name)
      {
        gathered_by_kernel = true;
        break;
      }
    if (gathered_by_kernel)
      continue;

    // skip if manage_state_advance is true and the variable is stateful (history_order > 0),
    // because in that case we will gather the variable on the compute device and do not need to set
    // up a gatherer for it
    if (getParam<bool>("manage_state_advance") && var->history_order() > 0)
      continue;

    _inputs.push_back({var->base_name(),
                       inferMOOSEIOType(var->base_name(), var->type()),
                       var->type(),
                       var->history_order()});
  }

  // User-specified mapping (overrides default mapping)
  const auto [input_types, inputs] =
      getInputParameterMapping<NEML2Utils::MOOSEIOType, std::string>("input_types", "inputs");

  for (auto i : index_range(inputs))
  {
    // Check if the input variable also appears in skip_input_variables
    const auto itr =
        std::find(_skip_input_variables.begin(), _skip_input_variables.end(), inputs[i]);
    if (itr != _skip_input_variables.end())
      paramError("skip_input_variables",
                 "The input variable ",
                 inputs[i],
                 " is listed in skip_input_variables, but it also appears in inputs. Please "
                 "remove it from either list.");
    // Check if the input variable exists in the NEML2 model
    if (model.input_variables().count(inputs[i]) == 0)
      paramError("inputs", "The neml2 input variable ", inputs[i], " does not exist.");
    // Check if the input variable is already gathered by a custom MOOSEToNEML2 object
    bool gathered_by_kernel = false;
    for (const auto & kernel_name : kernels)
      if (inputs[i] == kernel_name)
      {
        gathered_by_kernel = true;
        break;
      }
    if (gathered_by_kernel)
      paramError("inputs",
                 "The input variable ",
                 inputs[i],
                 " is listed in inputs, but it also appears in input_kernels. Please "
                 "remove it from either list.");
    // Check if the input variable is stateful and manage_state_advance is true
    if (getParam<bool>("manage_state_advance"))
      if (model.input_variable(inputs[i]).history_order() > 0)
        paramError(
            "inputs",
            "The input variable ",
            inputs[i],
            " is listed in inputs, but it is stateful (history_order > 0) and manage_state_advance "
            "is true. Please remove it from inputs, or set manage_state_advance to false.");

    // Get the existing mapping for this neml2 input variable and override it
    for (auto & input : _inputs)
      if (input.name == inputs[i])
      {
        input.moose_type = input_types[i];
        break;
      }
  }
}

void
NEML2Action::setupOutputMappings(const neml2::Model & model)
{
  if (!getParam<bool>("auto_output"))
    return;

  // Outputs
  for (const auto & [name, var] : model.output_variables())
    _outputs.push_back({name,
                        NEML2Utils::MOOSEIOType::MATERIAL,
                        var->type(),
                        /*history_order=*/0});
}

void
NEML2Action::setupParameterMappings(const neml2::Model & model)
{
  // User-specified mapping
  const auto [param_types, params] = getInputParameterMapping<NEML2Utils::MOOSEIOType, std::string>(
      "parameter_types", "parameters");

  for (auto i : index_range(params))
  {
    if (model.named_parameters().count(params[i]) == 0)
      paramError("parameters", "The neml2 parameter ", params[i], " does not exist.");
    const auto & param = model.get_parameter(params[i]);
    _params.push_back({params[i], inferMOOSEIOType(params[i], param.type()), param.type()});
  }
}

void
NEML2Action::setupDerivativeMappings(const neml2::Model & model)
{
  const auto derivs = getParam<std::vector<std::vector<std::string>>>("derivatives");

  for (auto i : index_range(derivs))
  {
    if (derivs[i].size() != 2)
      paramError("derivatives", "The length of each pair in derivatives must be 2.");
    if (model.output_variables().count(derivs[i][0]) == 0)
      paramError("derivatives", "The NEML2 output variable ", derivs[i][0], " does not exist.");
    if (model.input_variables().count(derivs[i][1]) == 0)
      paramError("derivatives", "The NEML2 input variable ", derivs[i][1], " does not exist.");

    const auto & y = derivs[i][0];
    const auto & x = derivs[i][1];
    const auto deriv_name = derivativePropertyNameFirst(y, x);
    _derivs.push_back({deriv_name, y, x});
  }
}

void
NEML2Action::setupParameterDerivativeMappings(const neml2::Model & model)
{
  const auto derivs = getParam<std::vector<std::vector<std::string>>>("parameter_derivatives");

  for (auto i : index_range(derivs))
  {
    if (derivs[i].size() != 2)
      paramError("parameter_derivatives",
                 "The length of each pair in parameter_derivatives must be 2.");
    if (model.output_variables().count(derivs[i][0]) == 0)
      paramError(
          "parameter_derivatives", "The NEML2 output variable ", derivs[i][0], " does not exist.");
    if (model.named_parameters().count(derivs[i][1]) == 0)
      paramError("parameter_derivatives", "The NEML2 parameter ", derivs[i][1], " does not exist.");

    const auto & y = derivs[i][0];
    const auto & x = derivs[i][1];
    const auto deriv_name = derivativePropertyNameFirst(y, x);
    _param_derivs.push_back({deriv_name, y, x});
  }
}

void
NEML2Action::printSummary() const
{
  if (!_app.parameters().have_parameter<bool>("parse_neml2_only"))
    return;

  // Save formatting of the output stream so that we can restore it later
  const auto flags = _console.flags();

  // Default width for the summary
  const int width = 79;

  _console << std::endl;
  _console << COLOR_CYAN << std::setw(width) << std::setfill('*') << std::left
           << "NEML2 MATERIAL MODEL SUMMARY BEGIN " << std::setfill(' ') << COLOR_DEFAULT
           << std::endl;

  // Metadata
  _console << "NEML2 input file location: " << fname() << std::endl;
  _console << "NEML2 action path:         " << parameters().blockFullpath() << std::endl;

  // List inputs, outputs, and parameters of the model
  _console << COLOR_CYAN << std::setw(width) << std::setfill('-') << std::left
           << "Material model structure " << std::setfill(' ') << COLOR_DEFAULT << std::endl;
  _console << *_model;

  // List transfer between MOOSE and NEML2
  if (!_app.parameters().get<bool>("parse_neml2_only"))
  {
    _console << COLOR_CYAN << std::setw(width) << std::setfill('-') << std::left
             << "Transfer between MOOSE and NEML2 " << std::setfill(' ') << COLOR_DEFAULT
             << std::endl;

    _console << "MOOSE --> NEML2" << std::endl;

    // List input transfer, MOOSE -> NEML2
    for (const auto & input : _inputs)
      _console << "  - " << (input.history_order > 0 ? ("(old) " + input.name) : input.name) << " ("
               << NEML2Utils::stringify(input.moose_type) << ")" << std::endl;

    // List parameter transfer, MOOSE -> NEML2
    for (const auto & param : _params)
      _console << "  - " << param.name << " (" << NEML2Utils::stringify(param.moose_type) << " --> "
               << param.neml2_type << ")" << std::endl;

    _console << "MOOSE <-- NEML2" << std::endl;

    // List output transfer, NEML2 -> MOOSE
    for (const auto & output : _outputs)
      _console << "  - " << output.name << std::endl;

    // List derivative transfer, NEML2 -> MOOSE
    for (const auto & deriv : _derivs)
      _console << "  - " << deriv.name << std::endl;

    // List parameter derivative transfer, NEML2 -> MOOSE
    for (const auto & param_deriv : _param_derivs)
      _console << "  - " << param_deriv.name << std::endl;
  }

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
  {
    auto n = input.name.size();
    if (input.history_order > 0)
      n += 6; // 6 is the length of "(old) "
    max_moose_name_length = std::max(max_moose_name_length, n);
  }
  for (const auto & param : _params)
    max_moose_name_length = std::max(max_moose_name_length, param.name.size());
  for (const auto & output : _outputs)
    max_moose_name_length = std::max(max_moose_name_length, output.name.size());
  for (const auto & deriv : _derivs)
    max_moose_name_length = std::max(max_moose_name_length, deriv.name.size());
  for (const auto & param_deriv : _param_derivs)
    max_moose_name_length = std::max(max_moose_name_length, param_deriv.name.size());
  return max_moose_name_length;
}
#endif // NEML2_ENABLED
