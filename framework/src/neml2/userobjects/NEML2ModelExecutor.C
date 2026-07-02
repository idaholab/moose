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
#include "neml2/csrc/aoti/Exception.h"

// parseLag / lagName / contains are shared with NEML2Action and live in NEML2Utils so the NEML2
// unity build does not see duplicate definitions.
using NEML2Utils::contains;
using NEML2Utils::lagName;
using NEML2Utils::parseLag;
#endif

registerMooseObject("MooseApp", NEML2ModelExecutor);

InputParameters
NEML2ModelExecutor::actionParams()
{
  auto params = emptyInputParameters();
  params.addParam<bool>(
      "manage_state_advance",
      false,
      "Keep state and forces on the device and advance it to old state and old forces without a "
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

    // there's no need to gather old values if we're managing state advance
    const auto [base_name, lag] = parseLag(uo.NEML2Name());
    if (_manage_state_advance && lag > 0)
      paramError("gatherers",
                 "The gatherer for old variable `",
                 uo.NEML2Name(),
                 "` is not needed when `manage_state_advance = true`.");

    addGatheredVariable(gatherer_name, uo.NEML2Name());
    _gatherers.push_back(&uo);
  }

  // deal with user object provided model parameters
  for (const auto & gatherer_name : getParam<std::vector<UserObjectName>>("param_gatherers"))
  {
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(gatherer_name, /*is_dependency=*/false);
    addGatheredParameter(gatherer_name, uo.NEML2Name());
    _param_gatherers.push_back(&uo);
  }

  // iterate over set of required inputs and error out if we find one that is not provided
  for (const auto & iname : model().input_names())
  {
    // if tensors are kept on device, we are not going to gather old values from moose
    if (_manage_state_advance && parseLag(iname).second > 0)
      continue;
    if (!_gathered_variable_names.count(iname))
      paramError("gatherers", "The required model input `", iname, "` is not gathered");
  }

  // Keep track of stateful (old) variables if manage_state_advance is true. NEML2 v3's eager
  // runtime requires every declared input to be present on each call (unlike v2, which silently
  // zeroed undefined inputs), so we seed each lagged state variable with a base-shaped zero tensor
  // (the correct initial "old" value; it broadcasts over the batch). After each converged step
  // advanceState() overwrites these with the cached current values.
  if (_manage_state_advance)
  {
    const auto & in_names = model().input_names();
    const auto & in_shapes = model().input_base_shapes();
    const auto opts = at::TensorOptions().dtype(at::kDouble).device(device());
    for (const auto i : index_range(in_names))
      if (parseLag(in_names[i]).second > 0)
        _state_vars[in_names[i]] = at::zeros(in_shapes[i], opts);
  }
}

std::size_t
NEML2ModelExecutor::getBatchIndex(dof_id_type elem_id) const
{
  return _batch_index_generator.getBatchIndex(elem_id);
}

void
NEML2ModelExecutor::addGatheredVariable(const UserObjectName & gatherer_name,
                                        const std::string & var)
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
    paramError("param_gatherers",
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

    // Send input variables to the compute device
    for (auto & [var, val] : _in)
      val = val.to(device());

    // Push the gathered model parameters into the NEML2 model (on the compute device) so the
    // subsequent evaluation and its parameter Jacobian use the MOOSE-provided values.
    for (auto & [pname, pval] : _model_params)
      model().set_parameter(pname, pval.to(device()));
    _model_params.clear();
  }
  catch (std::exception & e)
  {
    mooseError("An error occurred while filling inputs for the NEML2 model. Error message:\n",
               e.what(),
               NEML2Utils::NEML2_help_message);
  }
}

void
NEML2ModelExecutor::advanceState()
{
  if (!_manage_state_advance || _t_step == 0)
    return;

  for (const auto & [name, val] : _state_vars)
  {
    const auto [base_name, lag] = parseLag(name);
    mooseAssert(lag > 0, "Invalid lag for a stateful variable");
    // cache the value from the current step (favor output over input); the value that feeds
    // "base~lag" next step is the current "base~(lag-1)" (which is "base" itself for lag == 1).
    const auto curr_name = lagName(base_name, lag - 1);
    if (_out.count(curr_name))
      _state_vars[name] = _out.at(curr_name);
    else if (_in.count(curr_name))
      _state_vars[name] = _in.at(curr_name);
    else
      mooseError("Failed to find cached value for old variable: ", name);
  }
}

bool
NEML2ModelExecutor::solve()
{
  try
  {
    // Evaluate the NEML2 material model (value + Jacobian)
    TIME_SECTION("NEML2 solve", 3, "Solving NEML2 material model");

    auto [out, dout_din] = model().jacobian(_in);
    _out = std::move(out);
    _dout_din = std::move(dout_din);

    // Parameter Jacobian d(output)/d(parameter), only when some object requested it.
    // param_jacobian recomputes the (identical) outputs via reverse-mode AD over the model
    // parameters; the input chain rule above is independent of it.
    if (!_retrieved_parameter_derivatives.empty())
    {
      // .first repeats the (identical) outputs already obtained from jacobian(); keep only the
      // parameter-derivative blocks.
      auto param_jac = model().param_jacobian(_in);
      _dout_dparam = std::move(param_jac.second);
    }

    if (!_manage_state_advance)
      _in.clear();
  }
  catch (const neml2::aoti::Exception & e)
  {
    // NEML2 v3 distinguishes recoverable numerical failures (e.g. ConvergenceError: a Newton
    // divergence / max-iters) from non-recoverable ones (FatalError: shape / device / config). A
    // retry can only clear the former -- by cutting the time step -- so a non-recoverable error
    // must hard-fail instead of triggering a futile sequence of time-step cuts.
    if (!e.recoverable())
      mooseError("NEML2 model evaluation failed with a non-recoverable error:\n",
                 e.what(),
                 NEML2Utils::NEML2_help_message);

    _error_message = e.what();
    _error = true;
    if (_debug_inputs_on_failure)
    {
      std::ostringstream os;
      os << "\nNEML2 input variables:\n";
      for (const auto & iname : model().input_names())
      {
        os << "  - " << iname << ": ";
        const auto it = _in.find(iname);
        if (it == _in.end())
          os << "missing\n";
        else if (!it->second.defined())
          os << "undefined\n";
        else
        {
          const auto & val = it->second;
          os << "device=" << val.device() << " dtype=" << val.scalar_type()
             << " sizes=" << val.sizes();
          if (val.numel() > 0)
          {
            auto cpu = val.detach().to(val.options().device(at::kCPU));
            auto flat = cpu.reshape({-1});
            os << " min=" << flat.min().item<double>() << " max=" << flat.max().item<double>()
               << " mean=" << flat.mean().item<double>()
               << " nan=" << (at::isnan(flat).any().item<bool>() ? "true" : "false")
               << " inf=" << (at::isinf(flat).any().item<bool>() ? "true" : "false");
          }
          os << "\n";
        }
      }
      _error_message += os.str();
    }
  }
  catch (const std::exception & e)
  {
    // Not a NEML2 exception -- unexpected; do not silently retry it as a recoverable failure.
    mooseError("NEML2 model evaluation raised an unexpected error:\n",
               e.what(),
               NEML2Utils::NEML2_help_message);
  }

  return !_error;
}

void
NEML2ModelExecutor::extractOutputs()
{
  try
  {
    // retrieve outputs
    for (auto & [y, target] : _retrieved_outputs)
      target = _out[y].to(output_device());

    // clear output unless we need it for on-device state advance
    if (!_manage_state_advance)
      _out.clear();

    // retrieve derivatives J[y][x]
    for (auto & [y, dy] : _retrieved_derivatives)
      for (auto & [x, target] : dy)
      {
        const auto & source = _dout_din[y][x];
        if (source.defined())
          target = source.to(output_device());
      }

    // retrieve parameter derivatives P[y][p]
    for (auto & [y, dy] : _retrieved_parameter_derivatives)
      for (auto & [p, target] : dy)
      {
        const auto & source = _dout_dparam[y][p];
        if (source.defined())
          target = source.to(output_device());
      }

    // clear derivatives
    _dout_din.clear();
    _dout_dparam.clear();
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

const at::Tensor &
NEML2ModelExecutor::getOutput(const std::string & output_name) const
{
  checkExecutionStage();

  if (!contains(model().output_names(), output_name))
    mooseError("Trying to retrieve a non-existent NEML2 output variable '", output_name, "'.");

  return _retrieved_outputs[output_name];
}

const at::Tensor &
NEML2ModelExecutor::getOutputDerivative(const std::string & output_name,
                                        const std::string & input_name) const
{
  checkExecutionStage();

  if (!contains(model().output_names(), output_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 input variable '",
               input_name,
               "', but the NEML2 output variable does not exist.");

  if (!contains(model().input_names(), input_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 input variable '",
               input_name,
               "', but the NEML2 input variable does not exist.");

  return _retrieved_derivatives[output_name][input_name];
}

const at::Tensor &
NEML2ModelExecutor::getOutputParameterDerivative(const std::string & output_name,
                                                 const std::string & parameter_name) const
{
  checkExecutionStage();

  if (!contains(model().output_names(), output_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 model parameter '",
               parameter_name,
               "', but the NEML2 output variable does not exist.");

  if (!model().parameter_base_shapes().count(parameter_name))
    mooseError("Trying to retrieve the derivative of NEML2 output variable '",
               output_name,
               "' with respect to NEML2 model parameter '",
               parameter_name,
               "', but the NEML2 model parameter does not exist.");

  return _retrieved_parameter_derivatives[output_name][parameter_name];
}

#endif
