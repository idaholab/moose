//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveMonteCarloDecision.h"
#include "Sampler.h"
#include "DenseMatrix.h"

registerMooseObject("StochasticToolsApp", AdaptiveMonteCarloDecision);

InputParameters
AdaptiveMonteCarloDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Generic reporter which decides whether or not to accept a proposed "
                             "sample in Adaptive Monte Carlo type of algorithms.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "output_required",
      "output_required",
      "Modified value of the model output from this reporter class.");
  params.addParam<ReporterValueName>("inputs", "inputs", "Uncertain inputs to the model.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  return params;
}

AdaptiveMonteCarloDecision::AdaptiveMonteCarloDecision(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_required(declareValue<std::vector<Real>>("output_required")),
    _inputs(declareValue<std::vector<Real>>("inputs")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _sampler(getSampler("sampler")),
    _ais(dynamic_cast<const AdaptiveImportanceSampler *>(&_sampler)),
    _check_step(std::numeric_limits<int>::max())
{

  // Check whether the selected sampler is an adaptive sampler or not
  if (!_ais)
    paramError("sampler", "The selected sampler is not an adaptive sampler.");

  _inputs.resize(_sampler.getNumberOfCols());
  _prev_val.resize(_sampler.getNumberOfCols());

  // Initialize the required variables depending upon the type of adaptive Monte Carlo algorithm
  if (_ais)
  {
    _prev_val = _ais->getInitialValues();
    _prev_val_out = 1.0;
    _output_required.resize(1);
  }
}

void
AdaptiveMonteCarloDecision::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _step)
  {
    _check_step = _step;
    return;
  }

  /* Decision step to whether or not to accept the proposed sample by the sampler.
     This decision step changes with the type of adaptive Monte Carlo sampling algorithm. */
  if (_ais)
  {
    const Real tmp = _ais->getUseAbsoluteValue() ? std::abs(_output_value[0]) : _output_value[0];
    const bool output_limit_reached = tmp >= _ais->getOutputLimit();
    _output_required[0] = output_limit_reached ? 1.0 : 0.0;
    if (_step <= _ais->getNumSamplesTrain())
    {
      /* This is the training phase of the Adaptive Importance Sampling algorithm.
         Here, it is decided whether or not to accept a proposed sample by the
         AdaptiveImportanceSampler.C sampler depending upon the model output_value. */
      _inputs = output_limit_reached ? _sampler.getNextLocalRow() : _prev_val;
      if (output_limit_reached)
        _prev_val = _inputs;
      _prev_val_out = _output_required[0];
    }
    else
    {
      /* This is the sampling phase of the Adaptive Importance Sampling algorithm.
         Here, all proposed samples by the AdaptiveImportanceSampler.C sampler are accepted since
         the importance distribution traning phase is finished. */
      _inputs = _sampler.getNextLocalRow();
      _prev_val_out = tmp;
    }
  }
  _check_step = _step;
}
