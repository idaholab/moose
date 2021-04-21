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
#include "Normal.h"
#include "Distribution.h"
#include "AdaptiveMonteCarloUtils.h"

registerMooseObjectAliased("StochasticToolsApp", AdaptiveMonteCarloDecision, "AdaptiveMonteCarloDecision");
registerMooseObjectReplaced("StochasticToolsApp",
                            AdaptiveMonteCarloDecision,
                            "07/01/2020 00:00",
                            AdaptiveMonteCarloDecision);

InputParameters
AdaptiveMonteCarloDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  params += addReporterTypeParams<Real>("output");
  params += addReporterTypeParams<Real>("inputs");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");

  return params;
}

AdaptiveMonteCarloDecision::AdaptiveMonteCarloDecision(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _output(declareAdaptiveMonteCarloDecisionValues<Real>("output")),
    _inputs(declareAdaptiveMonteCarloDecisionValues<Real>("inputs")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{
  // MultiMooseEnum amcs("subset");
  _subset_out = declareAMCSStatistics<unsigned int>("subset");
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  {
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _subset = 0;
    _count = 0;
    _check_even = 0;
    _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
      _prev_val[i] = _sampler->parameters().get<std::vector<Real>>("initial_values")[i];
    _prev_val_out = 1.0;
  }
}

void
AdaptiveMonteCarloDecision::initialize()
{
}

void
AdaptiveMonteCarloDecision::execute()
{
  if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  {
    if (_step <= (_sampler->parameters().get<int>("num_samplessub")))
    {
      _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
      (*_subset_out[0]) = _subset;
      for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
      {
        (*_inputs[i]) = _sampler->getNextLocalRow()[i];
        _inputs_sto[i].push_back((*_inputs[i]));
      }
      (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
      _outputs_sto.push_back((*_output[0]));
    } else
    {
      _subset = std::floor((_step-1) / _sampler->parameters().get<int>("num_samplessub"));
      (*_subset_out[0]) = _subset;
      _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
      if (_subset > (std::floor((_step-2) /  _sampler->parameters().get<int>("num_samplessub"))))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        _output_sorted = AdaptiveMonteCarloUtils::sortOUTPUT(_outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
          _inputs_sorted[j] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j], _outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        }
        _output_limits.push_back(AdaptiveMonteCarloUtils::computeMIN(_output_sorted));
      }
      if (_count >= _count_max)
      {
        ++_ind_sto;
        _count = 0;
        // std::cout << "Here" << _ind_sto << std::endl;
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          _prev_val[i] = _inputs_sorted[i][_ind_sto];
        _prev_val_out = _output_sorted[_ind_sto];
      } else
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          _prev_val[i] = _inputs_sto[i][_inputs_sto[i].size()-1];
        _prev_val_out = _outputs_sto[_outputs_sto.size()-1];
      }
      ++_count;
      if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) >= _output_limits[_subset-1])
      {
        // std::cout << "Accepted" << std::endl;
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_inputs[i]) = _sampler->getNextLocalRow()[i];
          _inputs_sto[i].push_back((*_inputs[i]));
        }
        (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
        _outputs_sto.push_back((*_output[0]));
      } else
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_inputs[i]) =  _prev_val[i]; // _inputs_sorted[i][_ind_sto];
          _inputs_sto[i].push_back((*_inputs[i]));
        }
        (*_output[0]) = _prev_val_out; // _output_sorted[_ind_sto];
        _outputs_sto.push_back((*_output[0]));
      }
    }
  } else if (_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    if (_check_even != _step)
    {
      if (_step <= (_sampler->parameters().get<int>("num_samples_train")))
      {
        if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) < (_sampler->parameters().get<Real>("output_limit")))
        {
          for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          {
            (*_inputs[i]) = _prev_val[i];
          }
          (*_output[0]) = 0.0;
        } else
        {
          for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          {
            (*_inputs[i]) = _sampler->getNextLocalRow()[i];
            _prev_val[i] = (*_inputs[i]);
          }
          (*_output[0]) = 1.0;
          _prev_val_out = (*_output[0]);
        }
      } else
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          (*_inputs[i]) = _sampler->getNextLocalRow()[i];
        _prev_val_out = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
        if (_prev_val_out >= (_sampler->parameters().get<Real>("output_limit")))
          (*_output[0]) = 1.0;
        else
          (*_output[0]) =  0.0;
      }
    }
    _check_even = _step;
  }
}

template <typename T>
std::vector<T *>
AdaptiveMonteCarloDecision::declareAMCSStatistics(const std::string & statistics)
{
  std::vector<T *> data;
  data.push_back(&this->declareValueByName<T>(statistics, 0));
  return data;
}
