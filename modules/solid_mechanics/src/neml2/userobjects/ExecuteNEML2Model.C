//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecuteNEML2Model.h"
#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", ExecuteNEML2Model);

#ifndef NEML2_ENABLED
NEML2ObjectStubImplementationOpen(ExecuteNEML2Model, ElementUserObject);
NEML2ObjectStubParam(std::vector<UserObjectName>, "gather_uos");
NEML2ObjectStubParam(std::vector<UserObjectName>, "gather_param_uos");
NEML2ObjectStubImplementationClose(ExecuteNEML2Model, ElementUserObject);
#else

#include "MOOSEToNEML2.h"
#include "MOOSEToNEML2Parameter.h"
#include "neml2/misc/math.h"
#include <set>

InputParameters
ExecuteNEML2Model::validParams()
{
  auto params = NEML2ModelInterface<ElementUserObject>::validParams();
  params.addClassDescription("Execute the specified NEML2 model");

  // we need the user to explicitly list the UOs so we can set up a construction order independent
  // dependency chain
  params.addParam<std::vector<UserObjectName>>("gather_uos", "List of MOOSE*ToNEML2 user objects");

  params.addParam<std::vector<UserObjectName>>(
      "gather_param_uos", {}, "List of MOOSE*ToNEML2Parameter user objects");

  // time input
  params.addParam<std::string>("neml2_time", "forces/t", "(optional) NEML2 variable name for time");

  // allow user to explicit skip required input variables
  params.addParam<std::vector<std::string>>(
      "skip_variables",
      {},
      "List of NEML2 variables to skip when setting up the model input. If an input variable is "
      "skipped, its value will stay zero. If a required input variable is not skipped, an error "
      "will be raised.");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

ExecuteNEML2Model::ExecuteNEML2Model(const InputParameters & params)
  : NEML2ModelInterface<ElementUserObject>(params),
    _neml2_time(neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_time"))),
    _neml2_time_old(NEML2Utils::getOldName(_neml2_time)),
    _output_ready(false)
{
  const auto gather_uo_names = getParam<std::vector<UserObjectName>>("gather_uos");
  const auto gather_param_uo_names = getParam<std::vector<UserObjectName>>("gather_param_uos");

  // add user object dependencies by name (the UOs do not need to exist yet for this)
  for (const auto & uo_name : gather_uo_names)
    _depend_uo.insert(uo_name);

  for (const auto & uo_name : gather_param_uo_names)
    _depend_uo.insert(uo_name);

  for (const auto & output : model().output_axis().variable_names())
  {
    // reserve Batch tensors for each output
    _outputs[output] = neml2::Tensor();

    // reserve Batch tensors for each output derivative
    for (const auto & uo_name : gather_uo_names)
      _doutputs[std::make_pair(output, uo_name)] = neml2::Tensor();

    // reserve Batch tensors for each model parameter derivative
    for (const auto & param : model().named_parameters())
      _doutputs_dparams[std::make_pair(output, param.first)] = neml2::Tensor();
  }

  for (const auto & var_name : getParam<std::vector<std::string>>("skip_variables"))
    _skip_vars.insert(neml2::utils::parse<neml2::VariableName>(var_name));

  validateModel();
}

void
ExecuteNEML2Model::initialSetup()
{
  // Initialize the model with a dummy batch shape of 1
  initModel(1);
  _in = neml2::LabeledVector::zeros(1, {&model().input_axis()});

  // deal with the (old) time input
  if (model().input_axis().has_variable(_neml2_time))
    _provided_inputs.insert(_neml2_time);
  if (model().input_axis().has_variable(_neml2_time_old))
    _provided_inputs.insert(_neml2_time_old);
  if (isParamSetByUser("neml2_time") && !model().input_axis().has_variable(_neml2_time) &&
      !model().input_axis().has_variable(_neml2_time_old))
    paramError("neml2_time",
               "The provided model has no time input `",
               _neml2_time,
               "` nor old time input `",
               _neml2_time_old,
               "`, and so the parameter neml2_time is not needed");

  // deal with user object provided inputs
  for (const auto & uo_name : getParam<std::vector<UserObjectName>>("gather_uos"))
  {
    // gather coupled user objects late to ensure they are constructed. Do not add them as
    // dependencies (that's already done in the constructor).
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(uo_name, /*is_dependency = */ false);
    _gather_uos.push_back(&uo);

    addUOVariable(uo_name, uo.getNEML2Variable());
  }

  // deal with user object provided parameters
  for (const auto & uo_name : getParam<std::vector<UserObjectName>>("gather_param_uos"))
  {
    // gather coupled user objects late to ensure they are constructed. Do not add them as
    // dependencies (that's already done in the constructor).
    const auto & uo =
        getUserObjectByName<MOOSEToNEML2Parameter>(uo_name, /*is_dependency = */ false);
    _gather_param_uos.push_back(&uo);

    // check if requested parameter exist in the model
    if (!model().named_parameters().has_key(uo.getNEML2Parameter()))
      mooseError("Trying to set scalar-valued material property named '",
                 uo.getNEML2Parameter(),
                 "' in the UserObject '",
                 uo_name,
                 "'. But there is not such parameter in the NEML2 material model.");
  }

  std::set<neml2::VariableName> required_inputs = model().input_axis().variable_names();

  // iterate over set of required inputs and error out if we find one that is not provided
  for (const auto & input : required_inputs)
    if (!_skip_vars.count(input))
      if (!_provided_inputs.count(input) &&
          (input.start_with("forces") || input.start_with("old_forces") ||
           input.start_with("old_state")))
        paramError(
            "gather_uos", "No user object is providing the required model input `", input, "`");

  // if a variable is stateful, then it'd better been retrieved by someone! In theory that's not
  // sufficient for stateful data management, but that's the best we can do here without being toooo
  // restrictive.
  for (const auto & input : required_inputs)
    if (input.start_with("old_state") && !_retrieved_outputs.count(input.slice(1).prepend("state")))
      mooseError(
          "The NEML2 model requires a stateful input variable `",
          input,
          "`, but its state counterpart on the output axis has not been retrieved by any object. "
          "Therefore, there is no way to properly propagate the corresponding stateful data in "
          "time. The common solution to this problem is to add a NEML2-to-MOOSE transfer material "
          "such as those called `NEML2ToXXXMOOSEMaterialProperty`.");
}

void
ExecuteNEML2Model::addUOVariable(const UserObjectName & uo_name, const neml2::VariableName & uo_var)
{
  // check if the model actually has an input corresponding to this UO
  if (!model().input_axis().has_variable(uo_var))
    paramError("gather_uos",
               "The supplied user object `",
               uo_name,
               "` connects to a model input that does not exist (`",
               uo_var,
               "`).");

  // make sure the variable is not provided by other UOs
  if (_provided_inputs.count(uo_var))
    paramError("gather_uos",
               "The variable `",
               uo_var,
               "` gathered by UO '",
               uo_name,
               "' is already gathered by another UO.");

  _provided_inputs.insert(uo_var);
}

bool
ExecuteNEML2Model::shouldCompute()
{
  // NEML2 computes residual and Jacobian together at EXEC_LINEAR
  // There is no work to be done at EXEC_NONLINEAR **UNLESS** we are computing the Jacobian for
  // automatic scaling.
  if (_fe_problem.computingScalingJacobian())
    return true;

  if (_fe_problem.currentlyComputingResidualAndJacobian())
    return true;

  if (_fe_problem.currentlyComputingJacobian())
    return false;

  return true;
}

void
ExecuteNEML2Model::initialize()
{
  if (!shouldCompute())
    return;

  _elem_to_batch_index.clear();
  _elem_to_batch_index_cache = {libMesh::invalid_uint, 0};
  _batch_index = 0;
  _output_ready = false;
}

void
ExecuteNEML2Model::execute()
{
  if (!shouldCompute())
    return;

  _elem_to_batch_index[_current_elem->id()] = _batch_index;
  _batch_index += _qrule->n_points();
}

void
ExecuteNEML2Model::finalize()
{
  if (!shouldCompute())
    return;

  // check if all gather uos collected the same amount of data, an that is is consistent with the
  // element id to batch index map
  for (const auto uo : _gather_uos)
    if (uo->size() != _batch_index)
      mooseError("Inconsistent gathered data size in ", uo->name());

  try
  {
    auto batch_shape = neml2::TensorShape{neml2::Size(_batch_index)};

    // Reallocate the variable storage only when the batch shape has changed
    if (neml2::TensorShapeRef(batch_shape) != model().batch_sizes())
    {
      initModel(batch_shape);
      _in = neml2::LabeledVector::zeros(batch_shape, {&model().input_axis()});
    }

    setParameter();

    updateForces();

    if (_t_step > 0)
    {
      applyPredictor();

      solve();

      // update output views
      for (auto & pair : _outputs)
        pair.second = _out.base_index(pair.first);

      getParameterDerivative();

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
ExecuteNEML2Model::setParameter()
{
  // Set model parameters from the parameter UOs
  for (const auto & uo : _gather_param_uos)
  {
    auto batch_shape = neml2::TensorShape{neml2::Size(uo->size())};
    if (neml2::TensorShapeRef(batch_shape) != model().batch_sizes())
      mooseError("parameter batch shape ",
                 batch_shape,
                 " is inconsistent with model batch shape ",
                 model().batch_sizes());
    uo->insertIntoParameter(model());
  }
  // Request gradient for the properties that we request AD for
  for (auto & [out, name, batch_tensor_ptr] : _retrieved_parameter_derivatives)
  {
    auto param = neml2::Tensor(model().get_parameter(name));
    param.requires_grad_(true);
  }
}

void
ExecuteNEML2Model::getParameterDerivative()
{
  // output retrieved derivatives
  for (auto & [out, in, batch_tensor_ptr] : _retrieved_derivatives)
    *batch_tensor_ptr = _dout_din.base_index({out, in});

  // output retrieved output variable derivatives wrt parameters
  for (auto & [out, pname, batch_tensor_ptr] : _retrieved_parameter_derivatives)
  {
    auto param = neml2::Tensor(model().get_parameter(pname));
    *batch_tensor_ptr = neml2::math::jacrev(_out.base_index(out), param);
  }
}

void
ExecuteNEML2Model::updateForces()
{
  for (const auto & uo : _gather_uos)
  {
    mooseAssert(model().input_axis().has_variable(uo->getNEML2Variable()),
                "Variable gathered by the UO does not connect to a NEML2 input force variable");
    uo->insertIntoInput(_in);
  }

  NEML2Utils::set(
      // The input LabeledVector
      _in,
      // NEML2 variable accessors
      {_neml2_time, _neml2_time_old},
      // Pointer to the unbatched data
      _provided_inputs.count(_neml2_time) ? &_t : nullptr,
      _provided_inputs.count(_neml2_time_old) ? &_t_old : nullptr);
}

void
ExecuteNEML2Model::applyPredictor()
{
  // Set trial state variables (i.e., initial guesses).
  // Right now we hard-code to use the old state as the trial state.
  // TODO: implement other predictors
  if (model().input_axis().has_subaxis("state") && model().input_axis().has_subaxis("old_state"))
    _in.slice("state").fill(_in.slice("old_state"));
}

void
ExecuteNEML2Model::solve()
{
  // 1. Sync input onto the model's device
  // 2. Evaluate the NEML2 material model
  // 3. Sync output back onto CPU
  auto res = model().value_and_dvalue(_in.to(device()));
  _out = std::get<0>(res).to(torch::kCPU);
  _dout_din = std::get<1>(res).to(torch::kCPU);
}

void
ExecuteNEML2Model::threadJoin(const UserObject & uo)
{
  const auto & m2n = static_cast<const ExecuteNEML2Model &>(uo);

  // append and renumber maps
  for (const auto & [elem_id, batch_index] : m2n._elem_to_batch_index)
    _elem_to_batch_index[elem_id] = _batch_index + batch_index;

  _batch_index += m2n._batch_index;
}

std::size_t
ExecuteNEML2Model::getBatchIndex(dof_id_type elem_id) const
{
  // return cached map lookup if applicable
  if (_elem_to_batch_index_cache.first == elem_id)
    return _elem_to_batch_index_cache.second;

  // else, search the map
  const auto it = _elem_to_batch_index.find(elem_id);
  if (it == _elem_to_batch_index.end())
    mooseError("No batch index found for element id ", elem_id);
  _elem_to_batch_index_cache = *it;
  return it->second;
}

void
ExecuteNEML2Model::checkExecutionStage() const
{
  if (_fe_problem.startedInitialSetup())
    mooseError("NEML2 variable views must be retrieved during object construction. This is a code "
               "problem.");
}

const neml2::Tensor &
ExecuteNEML2Model::getOutputView(const neml2::VariableName & output_name) const
{
  checkExecutionStage();

  // find output_name in batch tensor map
  const auto it = _outputs.find(output_name);
  if (it == _outputs.end())
    mooseError("Requested output variable `",
               output_name,
               "` not found in ExecuteNEML2Model object `",
               name(),
               "`.");

  // save retrieved output variables so that we can later check if all stateful variables are
  // retrieved and stored in MOOSE datastructures.
  _retrieved_outputs.insert(output_name);

  return it->second;
}

const neml2::Tensor &
ExecuteNEML2Model::getOutputDerivativeView(const neml2::VariableName & output_name,
                                           const neml2::VariableName & input_name) const
{
  checkExecutionStage();

  const auto gather_uo_names = getParam<std::vector<UserObjectName>>("gather_uos");

  // look up user object name for the requested input
  for (const auto & uo_name : gather_uo_names)
  {
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(uo_name, /*is_dependency = */ false);
    if (uo.getNEML2Variable() == input_name)
    {
      // find output_name in batch tensor map
      const auto it = _doutputs.find(std::make_pair(output_name, uo_name));
      if (it == _doutputs.end())
        mooseError("Requested derivative of `",
                   output_name,
                   "` with respect to `",
                   input_name,
                   "` not found in ExecuteNEML2Model object `",
                   name(),
                   "`.");
      // save derivative as retrieved (we carefully cast constness away, which is ok, as the items
      // stored in _retrieved_derivatives will be used only in this object)
      _retrieved_derivatives.emplace(
          output_name, input_name, const_cast<neml2::Tensor *>(&it->second));

      // return reference to derivative tensor view
      return it->second;
    }
  }

  // no user object was a match
  mooseError("Requested input variable `",
             input_name,
             "` not found in ExecuteNEML2Model object `",
             name(),
             "`.");
}

const neml2::Tensor &
ExecuteNEML2Model::getOutputParameterDerivativeView(const neml2::VariableName & output_name,
                                                    const std::string & parameter_name) const
{
  checkExecutionStage();

  if (!model().named_parameters().has_key(parameter_name))
    mooseError("Trying to get derivative of ",
               output_name,
               " with respect to material property named ",
               parameter_name,
               ". But there is not such parameter in the NEML2 material model.");

  const auto it = _doutputs_dparams.find(std::make_pair(output_name, parameter_name));
  if (it == _doutputs_dparams.end())
    mooseError("Requested derivative of `",
               output_name,
               "` with respect to `",
               parameter_name,
               "` not found in any ExecuteNEML2Model object.");

  // save derivative as retrieved (we carefully cast constness away, which is ok, as the items
  // stored in _retrieved_parameter_derivatives will be used only in this object)
  _retrieved_parameter_derivatives.emplace(
      output_name, parameter_name, const_cast<neml2::Tensor *>(&it->second));

  // return reference to derivative tensor view
  return it->second;
}

#endif
