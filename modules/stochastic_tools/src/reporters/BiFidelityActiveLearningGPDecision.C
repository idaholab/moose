//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BiFidelityActiveLearningGPDecision.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", BiFidelityActiveLearningGPDecision);

InputParameters
BiFidelityActiveLearningGPDecision::validParams()
{
  InputParameters params = ActiveLearningGPDecision::validParams();
  params.addClassDescription("Perform active learning decision making in bi-fidelity modeling.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<ReporterName>("outputs_lf",
                                        "Value of the LF model output from the SubApp.");
  params.addParam<ReporterValueName>("lf_corrected", "lf_corrected", "GP-corrected LF prediciton.");
  return params;
}

BiFidelityActiveLearningGPDecision::BiFidelityActiveLearningGPDecision(
    const InputParameters & parameters)
  : ActiveLearningGPDecision(parameters),
    _sampler(getSampler("sampler")),
    _outputs_lf(getReporterValue<std::vector<Real>>("outputs_lf", REPORTER_MODE_DISTRIBUTED)),
    _lf_corrected(declareValue<std::vector<Real>>("lf_corrected",
                                                  std::vector<Real>(sampler().getNumberOfRows()))),
    _local_comm(_sampler.getLocalComm())
{
}

bool
BiFidelityActiveLearningGPDecision::facilitateDecision()
{
  for (dof_id_type i = 0; i < _inputs.size(); ++i)
  {
    _gp_mean[i] = _gp_eval.evaluate(_inputs[i], _gp_std[i]);
    _flag_sample[i] = !learningFunction(_outputs_lf_batch[i] + _gp_mean[i], _gp_std[i]);
    _lf_corrected[i] = _outputs_lf_batch[i] + _gp_mean[i];
  }

  for (const auto & fs : _flag_sample)
    if (!fs)
      return false;
  return true;
}

void
BiFidelityActiveLearningGPDecision::preNeedSample()
{
  _outputs_lf_batch = _outputs_lf;
  _local_comm.allgather(_outputs_lf_batch);
  // Accumulate inputs and outputs if we previously decided we needed a sample
  if (_t_step > 1 && _decision)
  {
    std::vector<Real> differences(_outputs_global.size());
    for (dof_id_type i = 0; i < _outputs_global.size(); ++i)
      differences[i] = _outputs_global[i] - _outputs_lf_batch[i];

    // Accumulate data into _batch members
    setupData(_inputs, differences);

    // Retrain if we are outside the training phase
    if (_t_step >= _n_train)
      _al_gp.reTrain(_inputs_batch, _outputs_batch);
  }

  // Gather inputs for the current step
  _inputs = _inputs_global;

  // Evaluate GP and decide if we need more data if outside training phase
  if (_t_step >= _n_train)
    _decision = facilitateDecision();
}

bool
BiFidelityActiveLearningGPDecision::needSample(const std::vector<Real> &,
                                               dof_id_type,
                                               dof_id_type global_ind,
                                               Real & val)
{
  if (!_decision)
    val = _outputs_lf_batch[global_ind] + _gp_mean[global_ind];
  return _decision;
}
