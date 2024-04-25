//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "BayesianGPrySampler.h"
#include "Distribution.h"

registerMooseObject("StochasticToolsApp", BayesianGPrySampler);

InputParameters
BayesianGPrySampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription("Fast Bayesian inference with the GPry algorithm by El Gammal et al. 2023: sampler step");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  params.addRequiredRangeCheckedParam<int>(
      "num_iterations",
      "num_iterations>0",
      "Total number of iterations to train the GPry surrogate.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_samples", "num_samples>0", "The maximum number of subApp calls in each iteration.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_tries",
      "num_tries>0",
      "Number of samples to propose in each iteration (not all are sent for subApp evals).");
  // params.addRequiredParam<UserObjectName>("nn_name", "The name of the neural network classifer.");
  // params.addRequiredParam<UserObjectName>("gp_name", "The name of the GP surrogate of log_posterior.");
  params.addRequiredParam<UserObjectName>("model", "The name of the neural network classifer.");
  return params;
}

BayesianGPrySampler::BayesianGPrySampler(const InputParameters & parameters)
  : Sampler(parameters),
    SurrogateModelInterface(this),
    TransientInterface(this),
    // _nn(getSurrogateModelByName(
    //     "nn_name")), // getSurrogateModel<LibtorchANNSurrogate>
    // _gp(getSurrogateModel<GaussianProcess>("gp_name")),
    // _is_sampling_completed(false),
    _num_iterations(getParam<int>("num_iterations")),
    _num_samples(getParam<unsigned int>("num_samples")),
    _num_tries(getParam<unsigned int>("num_tries")),
    _check_step(std::numeric_limits<int>::min())
{
  // const auto & model_names = getParam<std::vector<UserObjectName>>("model");
  // _model.reserve(model_names.size());
  // for (const auto & nm : model_names)
  //   _model.push_back(&getSurrogateModelByName(nm));

  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));
  setNumberOfRows(_num_samples);
  setNumberOfCols(_distributions.size());
  _inputs_all.resize(_num_tries, std::vector<Real>(_distributions.size()));
  _inputs_subapp.resize(_num_samples, std::vector<Real>(_distributions.size()));
  _gp_mean.resize(_num_tries);
  _gp_std.resize(_num_tries);
  _nn_outputs.resize(_num_samples); // _num_tries
  setNumberOfRandomSeeds(getParam<unsigned int>("num_random_seeds"));
}

void
BayesianGPrySampler::sampleSetUp(const Sampler::SampleMode /*mode*/)
{
  // If we've already done this step, skip
  if (_check_step == _t_step)
    return;
  _check_step = _t_step;

  // if (_is_sampling_completed)
  //   mooseError("Internal bug: the adaptive sampling is supposed to be completed but another sample "
  //              "has been requested.");

  // for (dof_id_type i = 0; i < _num_tries; ++i)
  // {
  //   for (dof_id_type j = 0; j < _distributions.size(); ++j)
  //     _inputs_all[i][j] = _distributions[j]->quantile(getRand(_t_step));
  // }

  for (dof_id_type i = 0; i < _num_samples; ++i)
  {
    for (dof_id_type j = 0; j < _distributions.size(); ++j)
      _inputs_subapp[i][j] = _distributions[j]->quantile(getRand(_t_step));
  }

  if (_t_step > 1)
  {
    // const auto & model_names =
    //     getSurrogateModel<LibtorchANNSurrogate> getParam<std::vector<UserObjectName>>("model");
    // _model.reserve(model_names.size());
    // for (const auto & nm : model_names)
    //   _model.push_back(&getSurrogateModelByName(nm));
    LibtorchANNSurrogate _model = getSurrogateModel<LibtorchANNSurrogate>("model");
    std::cout << "Here " << std::endl;
    for (dof_id_type i = 0; i < _num_tries; ++i)
    {
      _nn_outputs[i] = _model.evaluate(_inputs_subapp[i]);
      // _gp_mean[i] = _gp.evaluate(_inputs_all[i], _gp_std[i]);
    }
  }
  // std::cout << Moose::stringify(_nn_outputs) << std::endl;
  // check if we have finished the sampling
  // if (_t_step >= _num_iterations)
  //   _is_sampling_completed = true;
}

Real
BayesianGPrySampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _inputs_subapp[row_index][col_index];
}

#endif
