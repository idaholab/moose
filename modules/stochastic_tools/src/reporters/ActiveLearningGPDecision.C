//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActiveLearningGPDecision.h"
#include "Sampler.h"
#include "AdaptiveMonteCarloUtils.h"

#include <math.h>

registerMooseObject("StochasticToolsApp", ActiveLearningGPDecision);

InputParameters
ActiveLearningGPDecision::validParams()
{
  InputParameters params = ActiveLearningReporterTempl<Real>::validParams();
  params.addClassDescription(
      "Evaluates a GP surrogate model, determines its prediction quality, "
      "launches full model if GP prediction is inadequate, and retrains GP.");
  MooseEnum learning_function("Ufunction COV");
  params.addRequiredParam<MooseEnum>(
      "learning_function", learning_function, "The learning function for active learning.");
  params.addRequiredParam<Real>("learning_function_threshold", "The learning function threshold.");
  params.addParam<Real>("learning_function_parameter",
                        std::numeric_limits<Real>::max(),
                        "The learning function parameter.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addParam<ReporterValueName>("flag_sample", "flag_sample", "Flag samples.");
  params.addRequiredParam<int>("n_train", "Number of training steps.");
  params.addParam<ReporterValueName>("inputs", "inputs", "The inputs.");
  params.addParam<ReporterValueName>("gp_mean", "gp_mean", "The GP mean prediction.");
  params.addParam<ReporterValueName>("gp_std", "gp_std", "The GP standard deviation.");
  return params;
}

ActiveLearningGPDecision::ActiveLearningGPDecision(const InputParameters & parameters)
  : ActiveLearningReporterTempl<Real>(parameters),
    SurrogateModelInterface(this),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _learning_function(getParam<MooseEnum>("learning_function")),
    _learning_function_threshold(getParam<Real>("learning_function_threshold")),
    _learning_function_parameter(getParam<Real>("learning_function_parameter")),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcess>("gp_evaluator")),
    _flag_sample(declareValue<std::vector<bool>>(
        "flag_sample", std::vector<bool>(sampler().getNumberOfRows(), false))),
    _n_train(getParam<int>("n_train")),
    _inputs(declareValue<std::vector<std::vector<Real>>>(
        "inputs",
        std::vector<std::vector<Real>>(sampler().getNumberOfRows(),
                                       std::vector<Real>(sampler().getNumberOfCols())))),
    _gp_mean(
        declareValue<std::vector<Real>>("gp_mean", std::vector<Real>(sampler().getNumberOfRows()))),
    _gp_std(
        declareValue<std::vector<Real>>("gp_std", std::vector<Real>(sampler().getNumberOfRows()))),
    _decision(true),
    _inputs_global(getGlobalInputData()),
    _outputs_global(getGlobalOutputData())
{
  if (_learning_function == "Ufunction" &&
      !parameters.isParamSetByUser("learning_function_parameter"))
    paramError("learning_function",
               "The Ufunction requires the model failure threshold ('learning_function_parameter') "
               "to be specified.");
}

bool
ActiveLearningGPDecision::learningFunction(const Real & gp_mean, const Real & gp_std) const
{
  if (_learning_function == "Ufunction")
    return (std::abs(gp_mean - _learning_function_parameter) / gp_std) >
           _learning_function_threshold;
  else if (_learning_function == "COV")
    return (gp_std / std::abs(gp_mean)) < _learning_function_threshold;
  else
    mooseError("Invalid learning function ", std::string(_learning_function));
  return false;
}

void
ActiveLearningGPDecision::setupData(const std::vector<std::vector<Real>> & inputs,
                                    const std::vector<Real> & outputs)
{
  _inputs_batch.insert(_inputs_batch.end(), inputs.begin(), inputs.end());
  _outputs_batch.insert(_outputs_batch.end(), outputs.begin(), outputs.end());
}

bool
ActiveLearningGPDecision::facilitateDecision()
{
  for (dof_id_type i = 0; i < _inputs.size(); ++i)
  {
    _gp_mean[i] = _gp_eval.evaluate(_inputs[i], _gp_std[i]);
    _flag_sample[i] = !learningFunction(_gp_mean[i], _gp_std[i]);
  }

  for (const auto & fs : _flag_sample)
    if (!fs)
      return false;
  return true;
}

void
ActiveLearningGPDecision::preNeedSample()
{
  // Accumulate inputs and outputs if we previously decided we needed a sample
  if (_step > 1 && _decision)
  {
    // Accumulate data into _batch members
    setupData(_inputs, _outputs_global);

    // Retrain if we are outside the training phase
    if (_step >= _n_train)
      _al_gp.reTrain(_inputs_batch, _outputs_batch);
  }

  // Gather inputs for the current step
  _inputs = _inputs_global;

  // Evaluate GP and decide if we need more data if outside training phase
  if (_step >= _n_train)
    _decision = facilitateDecision();
}

bool
ActiveLearningGPDecision::needSample(const std::vector<Real> &,
                                     dof_id_type,
                                     dof_id_type global_ind,
                                     Real & val)
{
  if (!_decision)
    val = _gp_mean[global_ind];
  return _decision;
}
