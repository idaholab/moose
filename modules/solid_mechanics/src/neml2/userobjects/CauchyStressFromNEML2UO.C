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

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR};
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
}

void
CauchyStressFromNEML2UO::batchCompute()
{
  try
  {
    initModel(_input_data.size());

    // Allocate the input and output
    if (_t_step == 0)
    {
      _in = neml2::LabeledVector::zeros(_input_data.size(), {&model().input_axis()});
      _out = neml2::LabeledVector::zeros(_input_data.size(), {&model().output_axis()});
    }

    updateForces();

    if (_t_step > 0)
    {
      applyPredictor();

      solve();

      // Fill the NEML2 output back into the MOOSE output data
      for (const neml2::TorchSize i : index_range(_output_data))
      {
        std::get<0>(_output_data[i]) =
            NEML2Utils::toMOOSE<SymmetricRankTwoTensor>(_out(stress()).batch_index({i}));
        std::get<1>(_output_data[i]) = RankFourTensor(NEML2Utils::toMOOSE<SymmetricRankFourTensor>(
            _dout_din(stress(), strain()).batch_index({i})));
      }
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
