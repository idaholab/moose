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
NEML2ObjectStubImplementationClose(ExecuteNEML2Model, ElementUserObject);
#else

#include "MOOSEToNEML2.h"
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

  // time input
  params.addParam<std::string>("neml2_time", "forces/t", "NEML2 variable accessor for time");

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
    _output_ready(false),
    _in_out_allocated(false)
{
  const auto gather_uo_names = getParam<std::vector<UserObjectName>>("gather_uos");

  // add user object dependencies by name (the UOs do not need to exist yet for this)
  for (const auto & uo_name : gather_uo_names)
    _depend_uo.insert(uo_name);

  for (const auto & output : model().output_axis().variable_accessors(true))
  {
    // reserve Batch tensors for each output
    _outputs[output] = neml2::BatchTensor();

    // reserve Batch tensors for each output derivative
    for (const auto & uo_name : gather_uo_names)
      _doutputs[std::make_pair(output, uo_name)] = neml2::BatchTensor();
  }

  validateModel();
}

void
ExecuteNEML2Model::initialSetup()
{
  // set of provided and required inputs
  std::set<neml2::VariableName> _provided_inputs;

  // deal with the time input
  if (model().input_axis().has_variable(_neml2_time))
    _provided_inputs.insert(_neml2_time);
  else if (isParamSetByUser("neml2_time"))
    paramError("neml2_time", "The provided model has no time input `", _neml2_time, "`.");

  // deal with user object provided inputs
  for (const auto & uo_name : getParam<std::vector<UserObjectName>>("gather_uos"))
  {
    // gather coupled user objects late to ensure they are constructed. Do not add them as
    // dependencies (that's already done in the constructor).
    const auto & uo = getUserObjectByName<MOOSEToNEML2>(uo_name, /*is_dependency = */ false);
    _gather_uos.push_back(&uo);

    const auto uo_var = uo.getNEML2Variable();
    // check if the model actually has an input corresponding to this UO
    if (!model().input_axis().has_variable(uo_var))
    {
      paramError("gather_uos",
                 "The supplied user object `",
                 uo_name,
                 "` connects to a model input that does not exist (`",
                 uo_var,
                 "`).");
    }
    _provided_inputs.insert(uo_var);
  }

  // iterate over set of required inputs and error out if we find one that is not provided
  std::set<neml2::VariableName> _required_inputs = model().input_axis().variable_accessors(true);
  for (const auto & input : _required_inputs)
    if (!_provided_inputs.count(input) && input.start_with("forces"))
      paramWarning(
          "gather_uos", "No user object is providing the required model input `", input, "`");
}

void
ExecuteNEML2Model::timestepSetup()
{
  if (_t_step > 0)
    advanceStep();
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
  _elem_to_batch_index.clear();
  _elem_to_batch_index_cache = {libMesh::invalid_uint, 0};
  _batch_index = 0;

  if (shouldCompute())
    _output_ready = false;
}

void
ExecuteNEML2Model::execute()
{
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
    initModel(_batch_index);

    // Allocate the input and output once
    if (!_in_out_allocated)
    {
      _in = neml2::LabeledVector::zeros(_batch_index, {&model().input_axis()});
      _out = neml2::LabeledVector::zeros(_batch_index, {&model().output_axis()});
      _in_out_allocated = true;
    }

    // Steps before stress update
    // preCompute();

    updateForces();

    if (_t_step > 0)
    {
      applyPredictor();

      solve();

      // update output views
      for (auto & pair : _outputs)
        pair.second = _out(pair.first);

      // output retrieved derivatives
      for (auto & [out, in, batch_tensor_ptr] : _retrieved_derivatives)
        *batch_tensor_ptr = _dout_din(out, in);

      // Additional calculations after stress update
      // postCompute();
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
ExecuteNEML2Model::advanceStep()
{
  // Set old state variables and old forces
  if (model().input_axis().has_subaxis("old_state") && model().output_axis().has_subaxis("state"))
    _in.slice("old_state").fill(_out.slice("state"));
  if (model().input_axis().has_subaxis("old_forces") && model().input_axis().has_subaxis("forces"))
    _in.slice("old_forces").fill(_in.slice("forces"));
}

void
ExecuteNEML2Model::updateForces()
{
  for (const auto & uo : _gather_uos)
  {
    // we can do this check upfront
    if (model().input_axis().has_variable(uo->getNEML2Variable()))
      uo->insertIntoInput(_in);
    else
      mooseError("Unknown axis ", uo->getNEML2Variable());
  }

  NEML2Utils::set(
      // The input LabeledVector
      _in,
      // NEML2 variable accessors
      {_neml2_time},
      // Pointer to the unbatched data
      model().input_axis().has_variable(_neml2_time) ? &_t : nullptr);
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

const neml2::BatchTensor &
ExecuteNEML2Model::getOutputView(const neml2::VariableName & output_name) const
{
  // find output_name in batch tensor map
  const auto it = _outputs.find(output_name);
  if (it == _outputs.end())
    mooseError("Requested output variable `",
               output_name,
               "` not found in ExecuteNEML2Model object `",
               name(),
               "`.");
  return it->second;
}

const neml2::BatchTensor &
ExecuteNEML2Model::getOutputDerivativeView(const neml2::VariableName & output_name,
                                           const neml2::VariableName & input_name) const
{
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
          output_name, input_name, const_cast<neml2::BatchTensor *>(&it->second));

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

#endif
