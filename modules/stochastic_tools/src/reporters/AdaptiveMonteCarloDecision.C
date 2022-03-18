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
#include "AdaptiveMonteCarloUtils.h"
#include "StochasticToolsUtils.h"

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
    _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _sampler(getSampler("sampler")),
    _ais(dynamic_cast<const AdaptiveImportanceSampler *>(&_sampler)),
    _pss(dynamic_cast<const ParallelSubsetSimulation *>(&_sampler)),
    _check_step(std::numeric_limits<int>::max())
{

  // Check whether the selected sampler is an adaptive sampler or not
  if (!_ais && !_pss)
    paramError("sampler", "The selected sampler is not an adaptive sampler.");

  const auto rows = _sampler.getNumberOfRows();
  const auto cols = _sampler.getNumberOfCols();

  // Create communicator that only has processors with rows
  _communicator.split(
      _sampler.getNumberOfLocalRows() > 0 ? 1 : MPI_UNDEFINED, processor_id(), _local_comm);

  // Initialize the required variables depending upon the type of adaptive Monte Carlo algorithm
  _inputs.resize(cols, std::vector<Real>(rows));
  _prev_val.resize(cols, std::vector<Real>(rows));
  _output_required.resize(rows);
  _prev_val_out.resize(rows);

  if (_ais)
  {
    for (dof_id_type j = 0; j < _sampler.getNumberOfCols(); ++j)
      _prev_val[j][0] = _ais->getInitialValues()[j];
    _output_limit = _ais->getOutputLimit();
  }
  else if (_pss)
  {
    _inputs_sto.resize(cols, std::vector<Real>(_pss->getNumSamplesSub()));
    _inputs_sorted.resize(cols);
    _outputs_sto.resize(_pss->getNumSamplesSub());
    _output_limit = -std::numeric_limits<Real>::max();
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
    const bool output_limit_reached = tmp >= _output_limit;
    _output_required[0] = output_limit_reached ? 1.0 : 0.0;
    if (_step <= _ais->getNumSamplesTrain())
    {
      /* This is the training phase of the Adaptive Importance Sampling algorithm.
         Here, it is decided whether or not to accept a proposed sample by the
         AdaptiveImportanceSampler.C sampler depending upon the model output_value. */
      _inputs = output_limit_reached
                    ? StochasticTools::reshapeVector(_sampler.getNextLocalRow(), 1, true)
                    : _prev_val;
      if (output_limit_reached)
        _prev_val = _inputs;
      _prev_val_out = _output_required;
    }
    else
    {
      /* This is the sampling phase of the Adaptive Importance Sampling algorithm.
         Here, all proposed samples by the AdaptiveImportanceSampler.C sampler are accepted since
         the importance distribution traning phase is finished. */
      _inputs = StochasticTools::reshapeVector(_sampler.getNextLocalRow(), 1, true);
      _prev_val_out[0] = tmp;
    }
  }
  else if (_pss)
  {
    // Track the current subset
    const unsigned int subset =
        ((_step - 1) * _sampler.getNumberOfRows()) / _pss->getNumSamplesSub();
    const unsigned int sub_ind =
        (_step - 1) - (_pss->getNumSamplesSub() / _sampler.getNumberOfRows()) * subset;
    const unsigned int offset = sub_ind * _sampler.getNumberOfRows();
    const unsigned int count_max = 1 / _pss->getSubsetProbability();

    DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
    for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
    {
      const auto data = _sampler.getNextLocalRow();
      for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
        data_in(ss, j) = data[j];
    }
    _local_comm.sum(data_in.get_values());

    // Get the accepted samples outputs across all the procs from the previous step
    _output_required = (_pss->getUseAbsoluteValue())
                           ? AdaptiveMonteCarloUtils::computeVectorABS(_output_value)
                           : _output_value;
    _local_comm.allgather(_output_required);

    // These are the subsequent subsets which use Markov Chain Monte Carlo sampling scheme
    if (subset > 0)
    {
      if (sub_ind == 0)
      {
        // _output_sorted contains largest po percentile output values
        _output_sorted = AdaptiveMonteCarloUtils::sortOutput(
            _outputs_sto, _pss->getNumSamplesSub(), _pss->getSubsetProbability());
        // _inputs_sorted contains the input values corresponding to the largest po percentile
        // output values
        _inputs_sorted = AdaptiveMonteCarloUtils::sortInput(
            _inputs_sto, _outputs_sto, _pss->getNumSamplesSub(), _pss->getSubsetProbability());
        // Get the subset's intermediate failure threshold values
        _output_limit = AdaptiveMonteCarloUtils::computeMin(_output_sorted);
      }
      // Check whether the number of samples in a Markov chain exceeded the limit
      if (sub_ind % count_max == 0)
      {
        const unsigned int soffset = (sub_ind / count_max) * _sampler.getNumberOfRows();
        // Reinitialize the starting input values for the next set of Markov chains
        for (dof_id_type j = 0; j < _sampler.getNumberOfCols(); ++j)
          _prev_val[j].assign(_inputs_sorted[j].begin() + soffset,
                              _inputs_sorted[j].begin() + soffset + _sampler.getNumberOfRows());
        _prev_val_out.assign(_output_sorted.begin() + soffset,
                             _output_sorted.begin() + soffset + _sampler.getNumberOfRows());
      }
      else
      {
        // Otherwise, use the previously accepted input values to propose the next set of input
        // values
        for (dof_id_type j = 0; j < _sampler.getNumberOfCols(); ++j)
          _prev_val[j].assign(_inputs_sto[j].begin() + offset - _sampler.getNumberOfRows(),
                              _inputs_sto[j].begin() + offset);
        _prev_val_out.assign(_outputs_sto.begin() + offset - _sampler.getNumberOfRows(),
                             _outputs_sto.begin() + offset);
      }
    }

    // Check whether the outputs exceed the subset's intermediate failure threshold value
    for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
    {
      // Check whether the outputs exceed the subset's intermediate failure threshold value
      // If so, accept the proposed input values by the Sampler object
      // Otherwise, use the previously accepted input values
      const bool output_limit_reached = _output_required[ss] >= _output_limit;
      for (dof_id_type i = 0; i < _sampler.getNumberOfCols(); ++i)
      {
        _inputs[i][ss] = output_limit_reached ? data_in(ss, i) : _prev_val[i][ss];
        _inputs_sto[i][ss + offset] = _inputs[i][ss];
      }
      if (!output_limit_reached)
        _output_required[ss] = _prev_val_out[ss];
      _outputs_sto[ss + offset] = _output_required[ss];
    }
  }
  // Track the current step
  _check_step = _step;
}
