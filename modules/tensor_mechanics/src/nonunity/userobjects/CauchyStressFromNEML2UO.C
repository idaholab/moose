/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CauchyStressFromNEML2UO.h"

#ifdef NEML2_ENABLED
#include "NEML2Utils.h"
#endif // NEML2_ENABLED

registerMooseObject("BlackBearApp", CauchyStressFromNEML2UO);

InputParameters
CauchyStressFromNEML2UO::validParams()
{
  auto params = NEML2SolidMechanicsInterface<CauchyStressFromNEML2UOParent>::validParams();
  params.addClassDescription("Gather input variables required for an objective stress integration "
                             "from all quadrature points. The batched input vector is sent through "
                             "a NEML2 material model to perform the constitutive update.");
  params.addCoupledVar("temperature", "The temperature");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this userobject only at execute_on = LINEAR (i.e. during residual evaluation).
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

CauchyStressFromNEML2UO::CauchyStressFromNEML2UO(const InputParameters & params)
  : NEML2SolidMechanicsInterface<CauchyStressFromNEML2UOParent>(
        params, "mechanical_strain", "temperature")
{
#ifdef NEML2_ENABLED
  validateModel();
#endif // NEML2_ENABLED
}

#ifdef NEML2_ENABLED

void
CauchyStressFromNEML2UO::timestepSetup()
{
  if (_t_step > 0)
    advanceStep();
}

void
CauchyStressFromNEML2UO::batchCompute()
{
  try
  {
    // Allocate the input and output
    if (_t_step == 0)
    {
      _in = neml2::LabeledVector::zeros(_input_data.size(), {&model().input()});
      _out = neml2::LabeledVector::zeros(_input_data.size(), {&model().output()});
    }

    updateForces();

    if (_t_step > 0)
    {
      if (model().implicit())
        applyPredictor();

      solve();

      // Fill the NEML2 output back into the Blackbear output data
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
CauchyStressFromNEML2UO::advanceStep()
{
  // Set old state variables and old forces
  if (model().input().has_subaxis("old_state") && model().output().has_subaxis("state"))
    _in.slice("old_state").fill(_out.slice("state"));
  if (model().input().has_subaxis("old_forces") && model().input().has_subaxis("forces"))
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
      model().input().has_variable(strain()) ? &std::get<0>(input) : nullptr,
      model().input().has_variable(temperature()) ? &std::get<1>(input) : nullptr);

  NEML2Utils::set(
      // The input LabeledVector
      _in,
      // NEML2 variable accessors
      {time()},
      // Pointer to the unbatched data
      model().input().has_variable(time()) ? &_t : nullptr);
}

void
CauchyStressFromNEML2UO::applyPredictor()
{
  // Set trial state variables (i.e., initial guesses).
  // Right now we hard-code to use the old state as the trial state.
  // TODO: implement other predictors
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
