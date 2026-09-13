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
#include "NonlinearSystemBase.h"
#include <string>
#include <cctype>

#ifdef NEML2_ENABLED
#include <ATen/ATen.h>
#include "libmesh/id_types.h"
#include "neml2/csrc/aoti/Exception.h"

// torch::cuda::synchronize() (declared in <torch/cuda.h>, defined in libtorch) does a full device
// synchronization -- see deviceSynchronize(). We cannot include <torch/cuda.h> here: it lives under
// torch/csrc/api/include (not necessarily on MOOSE's include path), and the lower-level c10/cuda
// sync headers transitively include <cuda_runtime.h>, which is not on the include path either.
// Forward-declare the stable frontend symbol instead; it is resolved from libtorch (always linked)
// and is present even in a CPU-only torch, where deviceSynchronize() never reaches the call.
namespace torch
{
namespace cuda
{
void synchronize(int64_t device_index);
}
}

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
      "dump_inputs_on_failure",
      false,
      "When a NEML2 constitutive solve fails (recoverable convergence failure), serialize the "
      "input "
      "tensors to a per-rank TorchScript file '<model>_count<c>_rank<r>.pt' (c is a persistent "
      "failure counter). The file can be loaded offline with torch.jit.load to re-evaluate the "
      "model "
      "on the failing batch, independently of MOOSE.");
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

  // The executor evaluates on EXEC_LINEAR (residual), EXEC_NONLINEAR (Jacobian) and, for on-device
  // state advance, EXEC_TIMESTEP_END.
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
    _dump_inputs_on_failure(getParam<bool>("dump_inputs_on_failure")),
    _output_ready(false),
    // No derivative retained yet -> nothing stale; the retriever's undefined-derivative path (zero
    // fill) covers the first steps. Only an actual batch resize (meshChanged) makes it invalid.
    _derivative_valid(true),
    _error_message(""),
    _num_failed_dumps(0)
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

  // A NEML2 order-0 input that is ALSO a model output is an in-place-updated unknown (e.g. an
  // ImplicitUpdate initial guess with no old counterpart). Such an input, if gathered, would read
  // its own stale output material -- the previous converged step's value -- as the "current"
  // initial guess, warm-starting a rate-like unknown from a value that can prevent the return-map
  // Newton solve from converging. NEML2Action skips the default gatherer for these, so here we
  // cold-start each at a base-shaped zero (broadcast over the batch) injected on every evaluation.
  // An input the user mapped explicitly IS gathered, so the !gathered guard leaves that override
  // untouched.
  {
    const auto & in_names = model().input_names();
    const auto & in_shapes = model().input_base_shapes();
    const auto & out_names = model().output_names();
    const auto opts = at::TensorOptions().dtype(at::kDouble).device(device());
    for (const auto i : index_range(in_names))
      if (parseLag(in_names[i]).second == 0 && contains(out_names, in_names[i]) &&
          !_gathered_variable_names.count(in_names[i]))
        _zero_seed_vars[in_names[i]] = at::zeros(in_shapes[i], opts);
  }

  // iterate over set of required inputs and error out if we find one that is not provided
  for (const auto & iname : model().input_names())
  {
    // if tensors are kept on device, we are not going to gather old values from moose
    if (_manage_state_advance && parseLag(iname).second > 0)
      continue;
    // in-place-updated unknowns are cold-started at zero by the executor, not gathered
    if (_zero_seed_vars.count(iname))
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
  _output_ready = false;
  _error = false;
  _error_message.clear();
}

void
NEML2ModelExecutor::meshChanged()
{
  _output_ready = false;
  // The element-to-batch-index map is regenerated on a mesh change, resizing the batch. The
  // retained input derivative is now stale (sized to the old batch); invalidate it so execute()
  // recomputes it before any retriever reads it.
  _derivative_valid = false;
  if (_manage_state_advance)
    mooseError("The mesh changed while `manage_state_advance = true` for NEML2 model executor '",
               name(),
               "'. This mode requires a fixed mesh because state history is cached on the device.");
}

void
NEML2ModelExecutor::execute()
{
  // Nothing to recompute at EXEC_TIMESTEP_END: the value, parameter derivatives, and input Jacobian
  // are all retained from the solve (see below), so output/coupling consumers read the retained
  // tensors. The only job here is the optional on-device state advance after a converged step.
  if (_current_execute_flag == EXEC_TIMESTEP_END)
  {
    if (_manage_state_advance && _fe_problem.solverSystemConverged(/*sys_num=*/0))
      advanceState();
    return;
  }

  // If the batch is empty, we do not need to do anything
  if (_batch_index_generator.isEmpty())
    return;

  // Pin libtorch's intra-op thread count to this model's setting for the duration of the
  // evaluation, then restore it. MOOSE runs its element loops with libMesh threads and sets
  // libtorch to the same count (usually 1) globally; the NEML2 batched solve is a different
  // workload and may want a different count, controlled independently via the 'num_threads'
  // parameter.
  const NEML2Utils::ScopedNumThreads scoped_threads(neml2NumThreads());

  // MOOSE drives NEML2 exclusively in Float64; force the libtorch default dtype to Float64 around
  // the evaluation so any default-dtype-dependent tensor created while the model runs is Float64.
  const NEML2Utils::ScopedDefaultDtype scoped_dtype;

  fillInputs();

  if (_t_step > 0)
  {
    // Compute the (comparatively expensive) input Jacobian only when MOOSE is assembling the
    // Jacobian -- currentlyComputingJacobian() is set for a Jacobian evaluation, the automatic-
    // scaling Jacobian, and a combined residual-and-Jacobian evaluation. A pure residual evaluation
    // (line search, damping, JFNK residuals) takes the value-only path. (We cannot key off the
    // residual flag: the tagged residual path never sets currentlyComputingResidual().) Consumers
    // reading the input derivative at EXEC_TIMESTEP_END therefore see the last Jacobian's value.
    // Parameter derivatives are handled separately in solve()/extractOutputs(): because they are
    // typically consumed as outputs (e.g. inverse-optimization sensitivities), they are evaluated
    // on every pass so they are always current.
    //
    // Also recompute when the retained input derivative is no longer valid for the current batch
    // (e.g. after a mesh change resized the batch); otherwise a residual sweep would reuse a stale,
    // wrongly-sized derivative tensor and a retriever would index out of range.
    const bool compute_derivative = _fe_problem.currentlyComputingJacobian() || !_derivative_valid;

    auto success = solve(compute_derivative);
    if (success)
    {
      extractOutputs(compute_derivative);
      if (compute_derivative)
        _derivative_valid = true;
    }
  }
}

void
NEML2ModelExecutor::deviceSynchronize()
{
  if (device().is_cuda())
    torch::cuda::synchronize(device().index());
}

void
NEML2ModelExecutor::fillInputs()
{
  TIME_SECTION("NEML2::fillInputs", 1, "Gathering NEML2 inputs and copying them to the device");
  try
  {
    for (const auto & uo : _gatherers)
      uo->insertInto(_in);
    for (const auto & uo : _param_gatherers)
      uo->insertInto(_model_params);

    // Cold-start in-place-updated unknowns at zero (see initialSetup): a base-shaped zero broadcast
    // over the batch, re-injected on every evaluation so the return-map initial guess is 0 each
    // nonlinear iteration rather than the stale previous output value.
    for (const auto & [name, val] : _zero_seed_vars)
      _in[name] = val;

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

    // The H2D copies above are asynchronous on CUDA; block so this phase's time reflects the
    // transfer rather than leaking into the next synchronization point.
    deviceSynchronize();
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
NEML2ModelExecutor::solve(bool compute_derivative)
{
  try
  {
    // Evaluate the NEML2 material model. The value is always computed; the (comparatively
    // expensive) derivatives are computed selectively:
    //   - input Jacobian d(output)/d(input): only when MOOSE is assembling the Jacobian (compute_-
    //     derivative). A pure residual evaluation skips it.
    //   - parameter Jacobian d(output)/d(parameter): whenever any object requested it, on EVERY
    //   pass,
    //     because parameter derivatives are consumed as outputs (e.g. inverse-optimization
    //     sensitivities) and must be current whenever they are read.
    // Each of jacobian()/param_jacobian() also returns the value, so we only fall back to the
    // value-only forward() when neither derivative is needed.
    TIME_SECTION("NEML2::solve", 1, "Evaluating the NEML2 model");

    const bool compute_parameter_derivative = !_retrieved_parameter_derivatives.empty();

    if (compute_derivative)
    {
      auto [out, dout_din] = model().jacobian(_in);
      _out = std::move(out);
      _dout_din = std::move(dout_din);
    }
    if (compute_parameter_derivative)
    {
      // .first repeats the (identical) outputs; keep only the parameter-derivative blocks.
      auto [out, dout_dparam] = model().param_jacobian(_in);
      _out = std::move(out);
      _dout_dparam = std::move(dout_dparam);
    }
    if (!compute_derivative && !compute_parameter_derivative)
      _out = model().value(_in);

    // NEML2 launches its kernels asynchronously on CUDA; block here so the compute time is
    // attributed to this section instead of leaking into the output D2H copy in extractOutputs().
    deviceSynchronize();

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
  }
  catch (const std::exception & e)
  {
    // Not a NEML2 exception -- unexpected; do not silently retry it as a recoverable failure.
    mooseError("NEML2 model evaluation raised an unexpected error:\n",
               e.what(),
               NEML2Utils::NEML2_help_message);
  }

  // NOTE: a failed constitutive update is dumped in finalize(), not here. solve() is skipped on
  // ranks whose local batch is empty (see execute()'s isEmpty() early-return), so any collective
  // placed here would deadlock -- the empty-batch ranks would never reach it. finalize() runs
  // collectively on every rank, so the cross-rank failure counter lives there instead.
  return !_error;
}

std::string
NEML2ModelExecutor::failedInputDumpName(unsigned int count, const std::string & rank_token) const
{
  // Model name for the file; fall back to the object name when the AOTI folder input carries no
  // explicit model name. Sanitize to a safe filename fragment.
  std::string model_name = getParam<std::string>("model");
  if (model_name.empty())
    model_name = name();
  for (auto & c : model_name)
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-')
      c = '_';
  return model_name + "_count" + std::to_string(count) + "_rank" + rank_token + ".pt";
}

void
NEML2ModelExecutor::extractOutputs(bool compute_derivative)
{
  TIME_SECTION(
      "NEML2::extractOutputs", 1, "Copying NEML2 outputs and derivatives back to the host");
  try
  {
    // retrieve outputs. The D2H copies to output_device() (host) below are synchronous, so with
    // solve()'s post-compute sync in place this phase's time reflects the transfer alone.
    // .contiguous() so the consuming NEML2ToMOOSEMaterialProperty can read the batch with a plain
    // per-element memcpy off data_ptr() instead of per-qp at::Tensor ops.
    for (auto & [y, target] : _retrieved_outputs)
      target = _out[y].to(output_device()).contiguous();

    // clear output unless we need it for on-device state advance
    if (!_manage_state_advance)
      _out.clear();

    // Input derivatives are produced only on Jacobian evaluations (see solve()); a pure residual
    // evaluation leaves the previously retrieved input derivatives untouched -- they are not read
    // until the next Jacobian assembly, and consumers reading them for output at EXEC_TIMESTEP_END
    // see the last Jacobian's value.
    if (compute_derivative)
    {
      // retrieve derivatives J[y][x]
      for (auto & [y, dy] : _retrieved_derivatives)
        for (auto & [x, target] : dy)
        {
          const auto & source = _dout_din[y][x];
          if (source.defined())
            target = source.to(output_device()).contiguous();
        }
      _dout_din.clear();
    }

    // Parameter derivatives are computed on every pass they are requested (see solve()), so
    // retrieve them whenever any were requested -- they stay current for output/coupling consumers.
    if (!_retrieved_parameter_derivatives.empty())
    {
      // retrieve parameter derivatives P[y][p]
      for (auto & [y, dy] : _retrieved_parameter_derivatives)
        for (auto & [p, target] : dy)
        {
          const auto & source = _dout_dparam[y][p];
          if (source.defined())
            target = source.to(output_device()).contiguous();
        }
      _dout_dparam.clear();
    }
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
  // Remember this rank's own outcome before the collective below overwrites _error with the global
  // value -- only ranks that actually failed should write a dump file.
  const bool local_error = _error;

  // See if any rank failed
  processor_id_type pid;
  _communicator.maxloc(_error, pid);

  // Optionally capture the exact inputs of a failed constitutive update to per-rank TorchScript
  // files for offline re-evaluation. This lives in finalize() -- not solve() -- because solve() is
  // skipped on ranks whose local batch is empty (see execute()), so a collective there would
  // deadlock, whereas finalize() runs collectively on every rank. The counter is advanced on all
  // ranks in lockstep so it stays identical everywhere; since the console streams rank 0 only, the
  // reported message can then point at a single '<model>_count<c>_rank_*.pt' glob (one file per
  // failing rank).
  if (_dump_inputs_on_failure && _error)
  {
    const unsigned int count = _num_failed_dumps++; // advanced on all ranks -> stays consistent
    if (local_error)
      NEML2Utils::dumpInputsToTorchScript(
          _in, failedInputDumpName(count, std::to_string(processor_id())));
    const std::string exec = _fe_problem.getCurrentExecuteOnFlag().name();
    const unsigned int nl_it =
        _fe_problem.getNonlinearSystemBase(/*sys_num=*/0).getCurrentNonlinearIterationNumber();
    _error_message += "\nNEML2 constitutive update failed (step " + std::to_string(_t_step) +
                      ", execute_on=" + exec + ", nonlinear iteration " + std::to_string(nl_it) +
                      "). Input tensors dumped to " + failedInputDumpName(count, "*") +
                      " (one file per failing rank).";
  }

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
