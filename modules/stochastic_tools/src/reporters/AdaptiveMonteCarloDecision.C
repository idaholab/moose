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

  auto cols = _sampler.getNumberOfCols();

  // Initialize the required variables depending upon the type of adaptive Monte Carlo algorithm
  if (_ais)
  {
    _inputs.resize(1);
    _inputs[0].resize(cols);
    _prev_val.resize(1);
    _prev_val[0].resize(cols);
    _prev_val[0] = _ais->getInitialValues();
    _prev_val_out.resize(1);
    _prev_val_out[0] = 1.0;
    _output_required.resize(1);
  }
  else if (_pss)
  {
    auto rows = _sampler.getNumberOfRows();
    _inputs_sto.resize(cols);
    _inputs.resize(cols);
    _prev_val.resize(cols);
    for (dof_id_type i = 0; i < cols; ++i)
    {
      _inputs[i].resize(rows);
      _prev_val[i].resize(rows);
    }
    _inputs_sorted.resize(cols);
    _subset = 0;
    _count = 0;
    _prev_val_out.resize(rows);
    _output_required.resize(rows);
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
      _inputs[0] = output_limit_reached ? _sampler.getNextLocalRow() : _prev_val[0];
      if (output_limit_reached)
        _prev_val[0] = _inputs[0];
      _prev_val_out[0] = _output_required[0];
    }
    else
    {
      /* This is the sampling phase of the Adaptive Importance Sampling algorithm.
         Here, all proposed samples by the AdaptiveImportanceSampler.C sampler are accepted since
         the importance distribution traning phase is finished. */
      _inputs[0] = _sampler.getNextLocalRow();
      _prev_val_out[0] = tmp;
    }
  }
  else if (_pss)
  {
    std::vector<Real> data_in;
    data_in.resize(_sampler.getNumberOfRows());
    if (_step <= (int)(_pss->getNumSamplesSub() / _sampler.getNumberOfRows()))
    {
      _subset = std::floor((_step * _sampler.getNumberOfRows()) / _pss->getNumSamplesSub());
      for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
      {
        const auto data = _sampler.getNextLocalRow();
        data_in = data;
        _communicator.allgather(data_in);
      }
      for (dof_id_type ss = 0; ss < (_sampler.getNumberOfRows()); ++ss)
      {
        for (dof_id_type i = 0; i < _sampler.getNumberOfCols(); ++i)
        {
          _inputs_sto[i].push_back(data_in[(_sampler.getNumberOfCols()) * ss + i]);
          _inputs[i][ss] = data_in[(_sampler.getNumberOfCols()) * ss + i];
        }
      }
      _output_required = (_pss->getUseAbsoluteValue())
                             ? AdaptiveMonteCarloUtils::computeVectorABS(_output_value)
                             : _output_value;
      _communicator.allgather(_output_required);
      for (dof_id_type ss = 0; ss < _output_required.size(); ++ss)
        _outputs_sto.push_back(_output_required[ss]);
    }
    else
    {
      _subset = std::floor(((_step - 1) * _sampler.getNumberOfRows()) / _pss->getNumSamplesSub());
      _count_max = std::floor(1 / _pss->getSubsetProbability());
      if (_subset >
          (std::floor(((_step - 2) * _sampler.getNumberOfRows()) / _pss->getNumSamplesSub())))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        _output_sorted = AdaptiveMonteCarloUtils::sortOUTPUT(
            _outputs_sto, _pss->getNumSamplesSub(), _subset, _pss->getSubsetProbability());
        for (dof_id_type j = 0; j < _sampler.getNumberOfCols(); ++j)
        {
          _inputs_sorted[j].resize(
              std::floor(_pss->getNumSamplesSub() * _pss->getSubsetProbability()));
          _inputs_sorted[j] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j],
                                                                 _outputs_sto,
                                                                 _pss->getNumSamplesSub(),
                                                                 _subset,
                                                                 _pss->getSubsetProbability());
        }
        _output_limits.push_back(AdaptiveMonteCarloUtils::computeMIN(_output_sorted));
      }
      if (_count >= _count_max)
      {
        for (dof_id_type jj = 0; jj < _sampler.getNumberOfRows(); ++jj)
        {
          ++_ind_sto;
          for (dof_id_type k = 0; k < _sampler.getNumberOfCols(); ++k)
            _prev_val[k][jj] = _inputs_sorted[k][_ind_sto];
          _prev_val_out[jj] = _output_sorted[_ind_sto];
        }
        _count = 0;
      }
      else
      {
        for (dof_id_type jj = 0; jj < _sampler.getNumberOfRows(); ++jj)
        {
          for (dof_id_type k = 0; k < _sampler.getNumberOfCols(); ++k)
            _prev_val[k][jj] =
                _inputs_sto[k][_inputs_sto[k].size() - _sampler.getNumberOfRows() + jj];
          _prev_val_out[jj] = _outputs_sto[_outputs_sto.size() - _sampler.getNumberOfRows() + jj];
        }
      }
      ++_count;
      for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
      {
        const auto data = _sampler.getNextLocalRow();
        data_in = data;
        _communicator.allgather(data_in);
      }
      _output_required = (_pss->getUseAbsoluteValue())
                             ? AdaptiveMonteCarloUtils::computeVectorABS(_output_value)
                             : _output_value;
      _communicator.allgather(_output_required);
      std::vector<Real> Tmp2 = _output_required;
      for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
      {
        if (Tmp2[ss] >= _output_limits[_subset - 1])
        {
          for (dof_id_type i = 0; i < _sampler.getNumberOfCols(); ++i)
          {
            _inputs[i][ss] = data_in[(_sampler.getNumberOfCols()) * ss + i];
            _inputs_sto[i].push_back(_inputs[i][ss]);
          }
          _outputs_sto.push_back(Tmp2[ss]);
        }
        else
        {
          for (dof_id_type i = 0; i < _sampler.getNumberOfCols(); ++i)
          {
            _inputs[i][ss] = _prev_val[i][ss];
            data_in[(_sampler.getNumberOfCols()) * ss + i] = _inputs[i][ss];
            _inputs_sto[i].push_back(_inputs[i][ss]);
          }
          Tmp2[ss] = _prev_val_out[ss];
          _outputs_sto.push_back(Tmp2[ss]);
        }
      }
      _output_required = Tmp2;
    }
  }
  _check_step = _step;
}
