//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2ModelExecutor.h"
#include "MOOSEToNEML2.h"
#include "NEML2Utils.h"
#include <string>
#include <sstream>

#ifdef NEML2_ENABLED
#include <ATen/ATen.h>
#include "libmesh/id_types.h"
#include "neml2/tensors/functions/jacrev.h"
#include "neml2/dispatchers/ValueMapLoader.h"
#include "neml2/misc/string_utils.h"
#include "neml2/base/Settings.h"
#endif

registerMooseObject("MooseApp", NEML2ModelExecutor);

InputParameters
NEML2ModelExecutor::actionParams()
{
  auto params = emptyInputParameters();
  params.addParam<bool>(
      "manage_state_advance",
      false,
      "Keep state and forces on the device and advance it to old_state and old_forces without a "
      "roundtrip through MOOSE materials. This is only recommended for explicit time integration "
      "or when absolutely no restepping occurs (e.g. failed timesteps).");
  params.addParam<bool>(
      "debug_inputs_on_failure",
      false,
      "When a NEML2 solve fails, append a detailed dump of input tensors (defined/missing, "
      "shapes, and devices) to the error message.");
  return params;
}

InputParameters
NEML2ModelExecutor::validParams()
{
  auto params = NEML2ModelInterface<GeneralUserObject>::validParams();
  params += NEML2ModelExecutor::actionParams();
  params.addClassDescription("Execute the specified NEML2 model");

  params.addRequiredParam<UserObjectName>(
      "batch_index_generator",
      "The NEML2BatchIndexGenerator used to generate the element-to-batch-index map.");
  params.addParam<std::vector<UserObjectName>>(
      "gatherers",
      {},
      "List of MOOSE*ToNEML2 user objects gathering MOOSE data as NEML2 input variables");
  params.addParam<std::vector<UserObjectName>>(
      "param_gatherers",
      {},
      "List of MOOSE*ToNEML2 user objects gathering MOOSE data as NEML2 model parameters");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR, EXEC_TIMESTEP_END};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

NEML2ModelExecutor::NEML2ModelExecutor(const InputParameters & params)
  : NEML2ModelInterface<GeneralUserObject>(params)
#ifdef NEML2_ENABLED
    ,
    _batch_index_generator(getUserObject<NEML2BatchIndexGenerator>("batch_index_generator")),
    _manage_state_advance(getParam<bool>("manage_state_advance")),
    _debug_inputs_on_failure(getParam<bool>("debug_inputs_on_failure")),
    _output_ready(false),
    _error_message("")
#endif
{
#ifdef NEML2_ENABLED
  validateModel();

  // add user object dependencies by name (the UOs do not need to exist yet for this)
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("gatherers"))
    _depend_uo.insert(gatherer_name);
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("param_gatherers"))
    _depend_uo.insert(gatherer_name);
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

    // there's no need to gather old/older variables if we're managing state advance
    auto sep = model().settings().history_separator();
    auto [base_name, history_order] = neml2::parse_history(uo.NEML2Name(), sep);
    if (_manage_state_advance && history_order > 0)
      paramError("gatherers",
                 "The gatherer for history variable `",
                 uo.NEML2Name(),
                 "` is not needed when `manage_state_advance = true`.");

    addGatheredVariable(gatherer_name, uo.NEML2Name());
    _gatherers.push_back(&uo);
  }

  // deal with user object provided model parameters
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("param_gatherers"))
  {
    // gather coupled user objects late to ensure they are constructed. Do not add them as
    // dependencies (that's already done in the constructor).
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(gatherer_name, /*is_dependency=*/false);
    addGatheredParameter(gatherer_name, uo.NEML2Name());
    _param_gatherers.push_back(&uo);
  }

  // iterate over set of required inputs and error out if we find one that is not provided
  for (const auto & [iname, ivar] : model().input_variables())
  {
    // if tensors are kept on device, we are not going to gather old values from moose
    if (_manage_state_advance && ivar->history_order() > 0)
      continue;
    if (!_gathered_variable_names.count(iname))
      paramError("gatherers", "The required model input `", iname, "` is not gathered");
  }

  // keep track of stateful variables if manage_state_advance is true
  if (_manage_state_advance)
    for (const auto & [iname, ivar] : model().input_variables())
      if (ivar->history_order() > 0)
        _state_vars[iname] = neml2::Tensor();
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
  _error = false;
  _error_message.clear();
}

void
NEML2ModelExecutor::meshChanged()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  _output_ready = false;
  if (_manage_state_advance)
    mooseError("The mesh changed while `manage_state_advance = true` for NEML2 model executor '",
               name(),
               "'. This mode requires a fixed mesh because state history is cached on the device.");
}

void
NEML2ModelExecutor::execute()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  if (_current_execute_flag == EXEC_TIMESTEP_END)
  {
    if (_manage_state_advance && _fe_problem.solverSystemConverged(/*sys_num=*/0))
      advanceState();
    return;
  }

  // If the batch is empty, we do not need to do anything
  if (_batch_index_generator.isEmpty())
    return;

  fillInputs();

  if (_t_step > 0)
  {
    auto success = solve();
    if (success)
      extractOutputs();
  }
}

void
NEML2ModelExecutor::fillInputs()
{
  try
  {
    for (const auto & uo : _gatherers)
      uo->insertInto(_in);
    for (const auto & uo : _param_gatherers)
      uo->insertInto(_model_params);

    if (_manage_state_advance && _t_step > 0)
      for (const auto & [name, val] : _state_vars)
        if (val.defined())
          _in[name] = val;

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
  catch (std::exception & e)
  {
    mooseError("An error occurred while filling inputs for the NEML2 model. Error message:\n",
               e.what(),
               NEML2Utils::NEML2_help_message);
  }
}

void
NEML2ModelExecutor::expandInputs()
{
  // Figure out what our batch size is
  std::vector<neml2::Tensor> defined;
  for (const auto & [key, value] : _in)
    defined.push_back(value);
  const auto s = neml2::utils::broadcast_dynamic_sizes(defined);

  // Make all inputs conformal
  for (auto & [key, value] : _in)
    if (value.dynamic_sizes() != s)
      _in[key] = value.dynamic_unsqueeze(0).dynamic_expand(s);
}

void
NEML2ModelExecutor::advanceState()
{
  if (!_manage_state_advance || _t_step == 0)
    return;

  for (const auto & [name, val] : _state_vars)
  {
    auto sep = model().settings().history_separator();
    auto [base_name, order] = neml2::parse_history(name, sep);
    mooseAssert(order > 0, "Invalid history order");
    // cache value from the current step
    // favor output over input
    auto curr_name = order == 1 ? base_name : base_name + sep + std::to_string(order - 1);
    if (_out.count(curr_name))
      _state_vars[name] = _out.at(curr_name);
    else if (_in.count(curr_name))
      _state_vars[name] = _in.at(curr_name);
    else
      mooseError("Failed to find cached value for history variable: ", name);
  }
}

bool
NEML2ModelExecutor::solve()
{
  try
  {
    // Evaluate the NEML2 material model
    TIME_SECTION("NEML2 solve", 3, "Solving NEML2 material model");

    // NEML2 requires double precision
    auto prev_dtype = neml2::get_default_dtype();
    neml2::set_default_dtype(neml2::kFloat64);

    if (scheduler())
    {
      // We only need consistent batch sizes if we are using the dispatcher
      expandInputs();
      neml2::ValueMapLoader loader(_in, 0);
      std::tie(_out, _dout_din) = dispatcher()->run(loader);
    }
    else
      std::tie(_out, _dout_din) = model().value_and_dvalue(_in);
    if (!_manage_state_advance)
      _in.clear();

    // Restore the default dtype
    neml2::set_default_dtype(prev_dtype);
  }
  catch (std::exception & e)
  {
    _error_message = e.what();
    _error = true;
    if (_debug_inputs_on_failure)
    {
      auto shape_to_string = [](const neml2::TensorShapeRef & shape) -> std::string
      {
        std::ostringstream os;
        os << "(";
        for (std::size_t i = 0; i < shape.size(); ++i)
        {
          if (i)
            os << ", ";
          os << shape[i];
        }
        os << ")";
        return os.str();
      };

      std::ostringstream os;
      os << "\nNEML2 input variables:\n";
      for (const auto & [var, val] : model().input_variables())
      {
        os << "  - " << var << ": ";
        const auto it = _in.find(var);
        if (it == _in.end())
          os << "missing\n";
        else if (!it->second.defined())
          os << "undefined\n";
        else
        {
          const auto & val = it->second;
          const auto & v = model().input_variable(var);
          neml2::TensorShape expected;
          const auto & intmd_sizes = v.intmd_sizes();
          expected.insert(expected.end(), intmd_sizes.begin(), intmd_sizes.end());
          const auto & base_sizes = v.base_sizes();
          expected.insert(expected.end(), base_sizes.begin(), base_sizes.end());

          os << "device=" << val.device() << " dtype=" << val.scalar_type()
             << " sizes=" << shape_to_string(val.sizes())
             << " batch=" << shape_to_string(val.batch_sizes().concrete())
             << " expected_base=" << shape_to_string(expected);

          if (val.numel() > 0)
          {
            auto cpu = val.detach().to(val.options().device(at::kCPU));
            auto flat = cpu.reshape({-1});
            auto min = flat.min().item<double>();
            auto max = flat.max().item<double>();
            auto mean = flat.mean().item<double>();
            auto has_nan = at::isnan(flat).any().item<bool>();
            auto has_inf = at::isinf(flat).any().item<bool>();
            os << " min=" << min << " max=" << max << " mean=" << mean
               << " nan=" << (has_nan ? "true" : "false")
               << " inf=" << (has_inf ? "true" : "false");
          }

          os << "\n";
        }
      }

      if (_manage_state_advance)
      {
        os << "NEML2 stateful variables:\n";
        for (const auto & [var, cached_val] : _state_vars)
        {
          os << "  - " << var << ": ";
          const auto it_out = _out.find(var);
          const auto it_in = _in.find(var);
          if (it_out == _out.end() || it_in == _in.end())
            os << "missing\n";
          else
          {
            const auto it = it_out != _out.end() ? it_out : it_in;
            const auto & val = it->second;
            os << "device=" << val.device() << " dtype=" << val.scalar_type()
               << " sizes=" << shape_to_string(val.sizes())
               << " batch=" << shape_to_string(val.batch_sizes().concrete());

            if (val.numel() > 0)
            {
              auto cpu = val.detach().to(val.options().device(at::kCPU));
              auto flat = cpu.reshape({-1});
              auto min = flat.min().item<double>();
              auto max = flat.max().item<double>();
              auto mean = flat.mean().item<double>();
              auto has_nan = at::isnan(flat).any().item<bool>();
              auto has_inf = at::isinf(flat).any().item<bool>();
              os << " min=" << min << " max=" << max << " mean=" << mean
                 << " nan=" << (has_nan ? "true" : "false")
                 << " inf=" << (has_inf ? "true" : "false");
            }

            os << "\n";
          }
        }
      }
      _error_message += os.str();
    }
  }

  return !_error;
}

void
NEML2ModelExecutor::extractOutputs()
{
  try
  {
    const auto N = _batch_index_generator.getBatchIndex();

    // retrieve outputs
    for (auto & [y, target] : _retrieved_outputs)
      target = _out[y].to(output_device());

    // retrieve parameter derivatives
    for (auto & [y, dy] : _retrieved_parameter_derivatives)
      for (auto & [p, target] : dy)
        target = neml2::jacrev(_out[y],
                               model().get_parameter(p),
                               /*retain_graph=*/true,
                               /*create_graph=*/false,
                               /*allow_unused=*/false)
                     .to(output_device());

    // clear output unless we need it for on-device state advance
    if (!_manage_state_advance)
      _out.clear();

    // retrieve derivatives
    for (auto & [y, dy] : _retrieved_derivatives)
      for (auto & [x, target] : dy)
      {
        const auto & source = _dout_din[y][x];
        if (source.defined())
          target = source.to(output_device()).dynamic_expand({neml2::Size(N)});
      }

    // clear derivatives
    _dout_din.clear();
  }
  catch (std::exception & e)
  {
    mooseError("An error occurred while retrieving outputs from the NEML2 model. Error message:\n",
               e.what(),
               NEML2Utils::NEML2_help_message);
  }
}

void
NEML2ModelExecutor::finalize()
{
  if (!NEML2Utils::shouldCompute(_fe_problem))
    return;

  // See if any rank failed
  processor_id_type pid;
  _communicator.maxloc(_error, pid);

  // Fail the next nonlinear convergence check if any rank failed
  if (_error)
  {
    _communicator.broadcast(_error_message, pid);
    if (_communicator.rank() == 0)
    {
      std::string msg = "NEML2 model execution failed on at least one processor with ID " +
                        std::to_string(pid) + ". Error message:\n";
      msg += _error_message;
      if (_fe_problem.isTransient())
        msg += "\nTo recover, the solution will fail and then be re-attempted with a reduced time "
               "step.";
      _console << COLOR_YELLOW << msg << COLOR_DEFAULT << std::endl;
    }
    _fe_problem.setFailNextNonlinearConvergenceCheck();
  }
  else if (_t_step > 0)
    _output_ready = true;
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

  if (!model().output_variables().count(output_name))
    mooseError("Trying to retrieve a non-existent NEML2 output variable '", output_name, "'.");

  return _retrieved_outputs[output_name];
}

const neml2::Tensor &
NEML2ModelExecutor::getOutputDerivative(const neml2::VariableName & output_name,
                                        const neml2::VariableName & input_name) const
{
  checkExecutionStage();

  if (!model().output_variables().count(output_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 input variable '",
               input_name,
               "', but the NEML2 output variable does not exist.");

  if (!model().input_variables().count(input_name))
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

  if (!model().output_variables().count(output_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 model parameter '",
               parameter_name,
               "', but the NEML2 output variable does not exist.");

  if (model().named_parameters().count(parameter_name) != 1)
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 model parameter '",
               parameter_name,
               "', but the NEML2 model parameter does not exist.");

  return _retrieved_parameter_derivatives[output_name][parameter_name];
}

#endif
