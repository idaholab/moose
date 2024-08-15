//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CauchyStressFromNEML2UO.h"
#include "NEML2Utils.h"

#ifdef NEML2_ENABLED
#include "neml2/misc/math.h"
#endif

registerMooseObject("SolidMechanicsApp", CauchyStressFromNEML2UO);

InputParameters
CauchyStressFromNEML2UO::validParams()
{
  auto params = NEML2SolidMechanicsInterface<CauchyStressFromNEML2UOParent>::validParams();
  NEML2Utils::addClassDescription(
      params,
      "Gather input variables required for an objective stress integration "
      "from all quadrature points. The batched input vector is sent through "
      "a NEML2 material model to perform the constitutive update.");
  params.addCoupledVar("temperature", "The temperature");
  params.addParam<std::vector<std::string>>(
      "scalar_material_property_names",
      {},
      "Names of the material properties (gathered from MOOSE) whose values are used to set "
      "parameter values in the NEML2 material model.");
  params.addParam<std::vector<UserObjectName>>(
      "scalar_material_property_values",
      {},
      "The list of userobjects for scalar-valued material properties. These userobjects should be "
      "of type BatchPropertyDerivativeRankTwoTensorReal. The property values gathered by each of "
      "the userobject will be used as parameter values in the NEML2 material model, and the "
      "derivatives of the Cauchy stress w.r.t. each of the parameter will be set into the "
      "corresponding userobject.");

  params.addParam<MaterialPropertyName>(
      "mechanical_strain", "mechanical_strain", "Name of the mechanical strain material property.");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

#ifndef NEML2_ENABLED

CauchyStressFromNEML2UO::CauchyStressFromNEML2UO(const InputParameters & params)
  : NEML2SolidMechanicsInterface<CauchyStressFromNEML2UOParent>(
        params, "mechanical_strain", "temperature")
{
  NEML2Utils::libraryNotEnabledError(params);
}

#else

CauchyStressFromNEML2UO::CauchyStressFromNEML2UO(const InputParameters & params)
  : NEML2SolidMechanicsInterface<CauchyStressFromNEML2UOParent>(
        params, "mechanical_strain", "temperature")
{
  validateModel();

  // Initialize scalar-valued material property UOs
  const auto prop_names = getParam<std::vector<std::string>>("scalar_material_property_names");
  const auto prop_values = getParam<std::vector<UserObjectName>>("scalar_material_property_values");
  if (prop_names.size() != prop_values.size())
    mooseError("Each of scalar_material_property_names should correspond to a value in "
               "scalar_material_property_values. ",
               prop_names.size(),
               " names are present, while ",
               prop_values.size(),
               " values are supplied.");
  for (auto i : index_range(prop_names))
    _props[prop_names[i]] = const_cast<BatchPropertyDerivativeRankTwoTensorReal *>(
        &getUserObjectByName<BatchPropertyDerivativeRankTwoTensorReal>(prop_values[i]));
}

void
CauchyStressFromNEML2UO::initialSetup()
{
  // Initialize the model with a dummy batch shape of 1
  initModel(1);
  _in = neml2::LabeledVector::zeros(1, {&model().input_axis()});
  _out = neml2::LabeledVector::zeros(1, {&model().output_axis()});
}

void
CauchyStressFromNEML2UO::preCompute()
{
  // Set the parameter value using batch material from MOOSE
  for (auto && [name, prop] : _props)
  {
    if (!model().named_parameters().has_key(name))
      mooseError("Trying to set scalar-valued material property named ",
                 name,
                 ". But there is not such parameter in the NEML2 material model.");

    auto input = NEML2Utils::homogenizeBatchedTuple(prop->getInputData());
    auto pval = NEML2Utils::toNEML2Batched(std::get<0>(input));
    pval.requires_grad_(true);
    model().set_parameter(name, pval);
  }
}

bool
CauchyStressFromNEML2UO::shouldCompute()
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
CauchyStressFromNEML2UO::batchCompute()
{
  if (!shouldCompute())
    return;

  try
  {
    auto batch_shape = neml2::TensorShape{neml2::Size(_input_data.size())};

    // Reallocate the variable storage only when the batch shape has changed
    if (neml2::TensorShapeRef(batch_shape) != model().batch_sizes())
    {
      initModel(batch_shape);
      _in = neml2::LabeledVector::zeros(batch_shape, {&model().input_axis()});
      _out = neml2::LabeledVector::zeros(batch_shape, {&model().output_axis()});
    }

    // Steps before stress update
    preCompute();

    updateForces();

    if (_t_step > 0)
    {
      applyPredictor();

      solve();

      // Fill the NEML2 output back into the MOOSE output data
      for (const neml2::Size i : index_range(_output_data))
      {
        std::get<0>(_output_data[i]) =
            NEML2Utils::toMOOSE<SymmetricRankTwoTensor>(_out.base_index(stress()).batch_index({i}));
        std::get<1>(_output_data[i]) = RankFourTensor(NEML2Utils::toMOOSE<SymmetricRankFourTensor>(
            _dout_din.base_index({stress(), strain()}).batch_index({i})));
      }

      // Additional calculations after stress update
      postCompute();
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
CauchyStressFromNEML2UO::postCompute()
{
  for (auto && [name, prop] : _props)
  {
    // Extract the parameter derivative from NEML2
    auto param = neml2::Tensor(model().get_parameter(name));
    auto dstress_dparam = neml2::math::jacrev(_out.base_index(stress()), param);

    // Fill the NEML2 parameter derivative into MOOSE UO
    auto & dstress_dprop = prop->setOutputData();
    for (const neml2::Size i : index_range(dstress_dprop))
      dstress_dprop[i] =
          NEML2Utils::toMOOSE<SymmetricRankTwoTensor>(dstress_dparam.batch_index({i}));
  }
}

void
CauchyStressFromNEML2UO::timestepSetup()
{
  if (_t_step > 0)
    advanceStep();
}

void
CauchyStressFromNEML2UO::advanceStep()
{
  // Set old state variables and old forces
  if (model().input_axis().has_subaxis("old_state") && model().output_axis().has_subaxis("state"))
    _in.slice("old_state").fill(_out.slice("state"));
  if (model().input_axis().has_subaxis("old_forces") && model().input_axis().has_subaxis("forces"))
    _in.slice("old_forces").fill(_in.slice("forces"));
}

void
CauchyStressFromNEML2UO::updateForces()
{
  // Homogenize the input data
  auto input = NEML2Utils::homogenizeBatchedTuple(_input_data);

  // Update forces
  NEML2Utils::setBatched(
      // The input LabeledVector
      _in,
      // NEML2 variable accessors
      {strain(), temperature()},
      // Pointer to the batched data
      model().input_axis().has_variable(strain()) ? &std::get<0>(input) : nullptr,
      model().input_axis().has_variable(temperature()) ? &std::get<1>(input) : nullptr);

  NEML2Utils::set(
      // The input LabeledVector
      _in,
      // NEML2 variable accessors
      {time()},
      // Pointer to the unbatched data
      model().input_axis().has_variable(time()) ? &_t : nullptr);
}

void
CauchyStressFromNEML2UO::applyPredictor()
{
  // Set trial state variables (i.e., initial guesses).
  // Right now we hard-code to use the old state as the trial state.
  // TODO: implement other predictors
  if (model().input_axis().has_subaxis("state") && model().input_axis().has_subaxis("old_state"))
    _in.slice("state").fill(_in.slice("old_state"));
}

void
CauchyStressFromNEML2UO::solve()
{
  // 1. Sync input onto the model's device
  // 2. Evaluate the NEML2 material model
  // 3. Sync output back onto CPU
  auto res = model().value_and_dvalue(_in.to(device()));
  _out = std::get<0>(res).to(torch::kCPU);
  _dout_din = std::get<1>(res).to(torch::kCPU);
}

#endif // NEML2_ENABLED
