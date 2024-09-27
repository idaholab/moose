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
      NEML2Utils::docstring("List of NEML2 variables to skip when setting up the model input. If "
                            "an input variable is skipped, its value will stay zero. If a required "
                            "input variable is not skipped, an error will be raised."));
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
    _output_ready(false),
    _in(model().input_storage()),
    _out(model().output_storage()),
    _dout_din(model().derivative_storage())
#endif
{
#ifdef NEML2_ENABLED
  // add user object dependencies by name (the UOs do not need to exist yet for this)
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("gatherers"))
    _depend_uo.insert(gatherer_name);
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("param_gatherers"))
    _depend_uo.insert(gatherer_name);

  for (const auto & var_name : getParam<std::vector<std::string>>("skip_inputs"))
    _skip_vars.insert(neml2::utils::parse<neml2::VariableName>(var_name));

  validateModel();
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
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(gatherer_name, /*is_dependency = */ false);

    // introspect the NEML2 model to figure out if the gatherer UO is gathering for a NEML2 input
    // variable or for a NEML2 model parameter
    if (model().input_axis().has_variable(getNEML2VariableName(uo.NEML2Name())))
    {
      auto varname = getNEML2VariableName(uo.NEML2Name());
      if (varname.start_with("old_forces") || varname.start_with("old_state"))
        uo.setMode(MOOSEToNEML2::Mode::OLD_VARIABLE);
      else
        uo.setMode(MOOSEToNEML2::Mode::VARIABLE);
      addGatheredVariable(gatherer_name, uo.NEML2VariableName());
    }
    else
      mooseError("The MOOSEToNEML2 gatherer named '",
                 gatherer_name,
                 "' is gathering MOOSE data for a non-existent NEML2 input variable named '",
                 uo.NEML2Name(),
                 "'.");

    _gatherers.push_back(&uo);
  }

  // deal with user object provided model parameters
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("param_gatherers"))
  {
    // gather coupled user objects late to ensure they are constructed. Do not add them as
    // dependencies (that's already done in the constructor).
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(gatherer_name, /*is_dependency = */ false);

    // introspect the NEML2 model to figure out if the gatherer UO is gathering for a NEML2 input
    // variable or for a NEML2 model parameter
    if (model().named_parameters().has_key(uo.NEML2Name()))
    {
      uo.setMode(MOOSEToNEML2::Mode::PARAMETER);
      addGatheredParameter(gatherer_name, uo.NEML2ParameterName());
    }
    else
      mooseError("The MOOSEToNEML2 gatherer named '",
                 gatherer_name,
                 "' is gathering MOOSE data for a non-existent NEML2 model parameter named '",
                 uo.NEML2Name(),
                 "'.");

    _gatherers.push_back(&uo);
  }

  // iterate over set of required inputs and error out if we find one that is not provided
  std::set<neml2::VariableName> required_inputs = model().input_axis().variable_names();
  for (const auto & input : required_inputs)
  {
    if (_skip_vars.count(input))
      continue;
    if (input.start_with("state"))
      continue;
    if (!_gathered_variable_names.count(input))
      paramError("gatherers", "The required model input `", input, "` is not gathered");
  }

  // If a variable is stateful, then it'd better been retrieved by someone! In theory that's not
  // sufficient for stateful data management, but that's the best we can do here without being
  // overly restrictive.
  for (const auto & input : required_inputs)
    if (input.start_with("old_state") && !_retrieved_outputs.count(input.remount("state")))
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
    auto batch_shape = neml2::TensorShape{neml2::Size(_batch_index_generator.getBatchIndex())};

    // Reallocate the variable storage only when the batch shape has changed
    if (neml2::TensorShapeRef(batch_shape) != model().batch_sizes())
      initModel(batch_shape);

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
    uo->insertInto(model());

  // Request gradient for the model parameters that we request AD for
  for (auto & [deriv, tensor] : _retrieved_parameter_derivatives)
    model().get_parameter(deriv.second).requires_grad_(true);
}

void
NEML2ModelExecutor::applyPredictor()
{
  // Set trial state variables (i.e., initial guesses).
  // Right now we hard-code to use the old state as the trial state.
  // TODO: implement other predictors
  if (model().input_axis().has_subaxis("state") && model().input_axis().has_subaxis("old_state"))
    _in.slice("state").fill(_in.slice("old_state"));
}

void
NEML2ModelExecutor::solve()
{
  // Evaluate the NEML2 material model
  model().value_and_dvalue(_in);
}

void
NEML2ModelExecutor::extractOutputs()
{
  // retrieve outputs
  for (auto & [y, tensor] : _retrieved_outputs)
    tensor = _out.base_index(y).to(torch::kCPU);

  // retrieve derivatives
  for (auto & [deriv, tensor] : _retrieved_derivatives)
  {
    auto & [y, x] = deriv;
    tensor = _dout_din.base_index({y, x}).to(torch::kCPU);
  }

  // retrieve parameter derivatives
  for (auto & [deriv, tensor] : _retrieved_parameter_derivatives)
  {
    auto & [y, p] = deriv;
    tensor = neml2::math::jacrev(_out.base_index(y), model().get_parameter(p)).to(torch::kCPU);
  }
}

void
NEML2ModelExecutor::checkExecutionStage() const
{
  if (_fe_problem.startedInitialSetup())
    mooseError("NEML2 output variables and derivatives must be retrieved during object "
               "construction. This is a code problem.");
}

const neml2::Tensor &
NEML2ModelExecutor::getOutputView(const neml2::VariableName & output_name) const
{
  checkExecutionStage();

  if (!model().output_axis().has_variable(output_name))
    mooseError("Trying to retrieve a non-existent NEML2 output variable '", output_name, "'.");

  return _retrieved_outputs[output_name];
}

const neml2::Tensor &
NEML2ModelExecutor::getOutputDerivativeView(const neml2::VariableName & output_name,
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

  return _retrieved_derivatives[{output_name, input_name}];
}

const neml2::Tensor &
NEML2ModelExecutor::getOutputParameterDerivativeView(const neml2::VariableName & output_name,
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

  return _retrieved_parameter_derivatives[{output_name, parameter_name}];
}

#endif
