//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2ModelExecutor.h"
#include "MOOSEToNEML2.h"
#include <set>

#ifdef NEML2_ENABLED
#include "neml2/misc/math.h"
#endif

registerMooseObject("SolidMechanicsApp", NEML2ModelExecutor);

InputParameters
NEML2ModelExecutor::actionParams()
{
  auto params = NEML2ModelInterface<GeneralUserObject>::validParams();
  // allow user to explicit skip required input variables
  params.addParam<std::vector<std::string>>(
      "skip_inputs",
      {},
      NEML2Utils::docstring(
          "List of NEML2 variables to skip error checking when setting up the model input. If an "
          "input variable is skipped, its value will stay zero. If a required input variable is "
          "not skipped, an error will be raised."));
  return params;
}

InputParameters
NEML2ModelExecutor::validParams()
{
  auto params = NEML2ModelExecutor::actionParams();
  params.addClassDescription(NEML2Utils::docstring("Execute the specified NEML2 model"));

  params.addRequiredParam<UserObjectName>(
      "batch_index_generator",
      "The NEML2BatchIndexGenerator used to generate the element-to-batch-index map.");
  params.addParam<std::vector<UserObjectName>>(
      "gatherers",
      {},
      NEML2Utils::docstring(
          "List of MOOSE*ToNEML2 user objects gathering MOOSE data as NEML2 input variables"));
  params.addParam<std::vector<UserObjectName>>(
      "param_gatherers",
      {},
      NEML2Utils::docstring(
          "List of MOOSE*ToNEML2 user objects gathering MOOSE data as NEML2 model parameters"));

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

NEML2ModelExecutor::NEML2ModelExecutor(const InputParameters & params)
  : NEML2ModelInterface<GeneralUserObject>(params)
#ifdef NEML2_ENABLED
    ,
    _batch_index_generator(getUserObject<NEML2BatchIndexGenerator>("batch_index_generator")),
    _output_ready(false)
#endif
{
#ifdef NEML2_ENABLED
  validateModel();

  // add user object dependencies by name (the UOs do not need to exist yet for this)
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("gatherers"))
    _depend_uo.insert(gatherer_name);
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("param_gatherers"))
    _depend_uo.insert(gatherer_name);

  // variables to skip error checking (converting vector to set to prevent duplicate checks)
  for (const auto & var_name : getParam<std::vector<std::string>>("skip_inputs"))
    _skip_vars.insert(NEML2Utils::parseVariableName(var_name));
#endif
}

#ifdef NEML2_ENABLED
void
NEML2ModelExecutor::initialSetup()
{
  // deal with user object provided inputs
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("gatherers"))
  {
    // gather coupled user objects late to ensure they are constructed. Do not add them as
    // dependencies (that's already done in the constructor).
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(gatherer_name, /*is_dependency=*/false);

    // the target neml2 variable must exist on the input axis
    if (!model().input_axis().has_variable(NEML2Utils::parseVariableName(uo.NEML2Name())))
      mooseError("The MOOSEToNEML2 gatherer named '",
                 gatherer_name,
                 "' is gathering MOOSE data for a non-existent NEML2 input variable named '",
                 uo.NEML2Name(),
                 "'.");

    // tell the gatherer to gather for a model input variable
    const auto varname = NEML2Utils::parseVariableName(uo.NEML2Name());
    if (varname.is_old_force() || varname.is_old_state())
      uo.setMode(MOOSEToNEML2::Mode::OLD_VARIABLE);
    else
      uo.setMode(MOOSEToNEML2::Mode::VARIABLE);

    addGatheredVariable(gatherer_name, uo.NEML2VariableName());
    _gatherers.push_back(&uo);
  }

  // deal with user object provided model parameters
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("param_gatherers"))
  {
    // gather coupled user objects late to ensure they are constructed. Do not add them as
    // dependencies (that's already done in the constructor).
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(gatherer_name, /*is_dependency=*/false);

    // introspect the NEML2 model to figure out if the gatherer UO is gathering for a NEML2 input
    // variable or for a NEML2 model parameter
    if (!model().named_parameters().has_key(uo.NEML2Name()))
      mooseError("The MOOSEToNEML2 gatherer named '",
                 gatherer_name,
                 "' is gathering MOOSE data for a non-existent NEML2 model parameter named '",
                 uo.NEML2Name(),
                 "'.");

    // tell the gatherer to gather for a model parameter
    uo.setMode(MOOSEToNEML2::Mode::PARAMETER);

    addGatheredParameter(gatherer_name, uo.NEML2ParameterName());
    _gatherers.push_back(&uo);
  }

  // iterate over set of required inputs and error out if we find one that is not provided
  std::vector<neml2::VariableName> required_inputs = model().input_axis().variable_names();
  for (const auto & input : required_inputs)
  {
    if (_skip_vars.count(input))
      continue;
    // skip input state variables because they are "initial guesses" to the nonlinear system
    if (input.is_state())
      continue;
    if (!_gathered_variable_names.count(input))
      paramError("gatherers", "The required model input `", input, "` is not gathered");
  }

  // If a variable is stateful, then it'd better been retrieved by someone! In theory that's not
  // sufficient for stateful data management, but that's the best we can do here without being
  // overly restrictive.
  for (const auto & input : required_inputs)
    if (input.is_old_state() && !_retrieved_outputs.count(input.current()))
      mooseError(
          "The NEML2 model requires a stateful input variable `",
          input,
          "`, but its state counterpart on the output axis has not been retrieved by any object. "
          "Therefore, there is no way to properly propagate the corresponding stateful data in "
          "time. The common solution to this problem is to add a NEML2ToMOOSE retriever such as "
          "those called `NEML2To*MOOSEMaterialProperty`.");
}

std::size_t
NEML2ModelExecutor::getBatchIndex(dof_id_type elem_id) const
{
  return _batch_index_generator.getBatchIndex(elem_id);
}

void
NEML2ModelExecutor::addGatheredVariable(const UserObjectName & gatherer_name,
                                        const neml2::VariableName & var)
{
  if (_gathered_variable_names.count(var))
    paramError("gatherers",
               "The NEML2 input variable `",
               var,
               "` gathered by UO '",
               gatherer_name,
               "' is already gathered by another gatherer.");
  _gathered_variable_names.insert(var);
}

void
NEML2ModelExecutor::addGatheredParameter(const UserObjectName & gatherer_name,
                                         const std::string & param)
{
  if (_gathered_parameter_names.count(param))
    paramError("gatherers",
               "The NEML2 model parameter `",
               param,
               "` gathered by UO '",
               gatherer_name,
               "' is already gathered by another gatherer.");
  _gathered_parameter_names.insert(param);
}

void
NEML2ModelExecutor::initialize()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  _output_ready = false;
}

void
NEML2ModelExecutor::execute()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  try
  {
    fillInputs();

    if (_t_step > 0)
    {
      applyPredictor();
      solve();
      extractOutputs();

      _output_ready = true;
    }
  }
  catch (neml2::NEMLException & e)
  {
    mooseException("An error occurred during the evaluation of a NEML2 model:\n",
                   e.what(),
                   NEML2Utils::NEML2_help_message);
  }
  catch (std::runtime_error & e)
  {
    mooseException("An unknown error occurred during the evaluation of a NEML2 model:\n",
                   e.what(),
                   "\nIt is possible that this error is related to NEML2.",
                   NEML2Utils::NEML2_help_message);
  }
}

void
NEML2ModelExecutor::fillInputs()
{
  for (const auto & uo : _gatherers)
    uo->insertInto(_in, _model_params);

  // Send input variables and parameters to device
  for (auto & [var, val] : _in)
    val = val.to(device());
  for (auto & [param, pval] : _model_params)
    pval = pval.to(device());

  // Update model parameters
  model().set_parameters(_model_params);
  _model_params.clear();

  // Request gradient for the model parameters that we request AD for
  for (const auto & [y, dy] : _retrieved_parameter_derivatives)
    for (const auto & [p, tensor] : dy)
      model().get_parameter(p).requires_grad_(true);
}

void
NEML2ModelExecutor::applyPredictor()
{
  if (!model().input_axis().has_state())
    return;

  // Set trial state variables (i.e., initial guesses).
  // Right now we hard-code to use the old state as the trial state.
  // TODO: implement other predictors
  const auto input_state = model().input_axis().subaxis(neml2::STATE);
  const auto input_old_state = model().input_axis().subaxis(neml2::OLD_STATE);
  for (const auto & var : input_state.variable_names())
    if (input_old_state.has_variable(var))
      _in[var.prepend(neml2::STATE)] = _in[var.prepend(neml2::OLD_STATE)];
}

void
NEML2ModelExecutor::solve()
{
  // Evaluate the NEML2 material model
  std::tie(_out, _dout_din) = model().value_and_dvalue(_in);
  _in.clear();
}

void
NEML2ModelExecutor::extractOutputs()
{
  const auto N = _batch_index_generator.getBatchIndex();

  // retrieve outputs
  for (auto & [y, target] : _retrieved_outputs)
    target = _out[y].to(torch::kCPU);

  // retrieve parameter derivatives
  for (auto & [y, dy] : _retrieved_parameter_derivatives)
    for (auto & [p, target] : dy)
      target = neml2::math::jacrev(_out[y],
                                   model().get_parameter(p),
                                   /*retain_graph=*/true,
                                   /*create_graph=*/false,
                                   /*allow_unused=*/false)
                   .to(torch::kCPU);

  // clear output
  _out.clear();

  // retrieve derivatives
  for (auto & [y, dy] : _retrieved_derivatives)
    for (auto & [x, target] : dy)
    {
      const auto & source = _dout_din[y][x];
      if (source.defined())
        target = source.to(torch::kCPU).batch_expand({neml2::Size(N)});
    }

  // clear derivatives
  _dout_din.clear();
}

void
NEML2ModelExecutor::checkExecutionStage() const
{
  if (_fe_problem.startedInitialSetup())
    mooseError("NEML2 output variables and derivatives must be retrieved during object "
               "construction. This is a code problem.");
}

const neml2::Tensor &
NEML2ModelExecutor::getOutput(const neml2::VariableName & output_name) const
{
  checkExecutionStage();

  if (!model().output_axis().has_variable(output_name))
    mooseError("Trying to retrieve a non-existent NEML2 output variable '", output_name, "'.");

  return _retrieved_outputs[output_name];
}

const neml2::Tensor &
NEML2ModelExecutor::getOutputDerivative(const neml2::VariableName & output_name,
                                        const neml2::VariableName & input_name) const
{
  checkExecutionStage();

  if (!model().output_axis().has_variable(output_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 input variable '",
               input_name,
               "', but the NEML2 output variable does not exist.");

  if (!model().input_axis().has_variable(input_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 input variable '",
               input_name,
               "', but the NEML2 input variable does not exist.");

  return _retrieved_derivatives[output_name][input_name];
}

const neml2::Tensor &
NEML2ModelExecutor::getOutputParameterDerivative(const neml2::VariableName & output_name,
                                                 const std::string & parameter_name) const
{
  checkExecutionStage();

  if (!model().output_axis().has_variable(output_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 model parameter '",
               parameter_name,
               "', but the NEML2 output variable does not exist.");

  if (!model().named_parameters().has_key(parameter_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 model parameter '",
               parameter_name,
               "', but the NEML2 model parameter does not exist.");

  return _retrieved_parameter_derivatives[output_name][parameter_name];
}

#endif
