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

registerMooseObject("StochasticToolsApp", AdaptiveMonteCarloDecision);

InputParameters
AdaptiveMonteCarloDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Generic reporter which decides whether or not to accept a proposed "
                             "sample in Adaptive Monte Carlo type of algorithms.");
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
  // Get the sampler object
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));

  // Initialize the required variables depending upon the type of adaptive Monte Carlo algorithm
  if (_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    _prev_val.resize(
        _sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    for (unsigned int i = 0;
         i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size();
         ++i)
      _prev_val[i] = _sampler->parameters().get<std::vector<Real>>("initial_values")[i];
    _prev_val_out = 1.0;
  }
}

void
AdaptiveMonteCarloDecision::execute()
{
  /* Decision step to whether or not to accept the proposed sample by the sampler.
     This decision step changes with the type of adaptive Monte Carlo sampling algorithm.*/
  if (_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    if (_check_step != _step)
    {
      if (_step <= (_sampler->parameters().get<int>("num_samples_train")))
      {
        /* This is the training phase of the Adaptive Importance Sampling algorithm.
           Here, it is decided whether or not to accept a proposed sample by the AIS.C sampler
           depending upon the model output.*/
        if (((_sampler->parameters().get<bool>("use_absolute_value"))
                 ? std::abs(*_output[0])
                 : (*_output[0])) < (_sampler->parameters().get<Real>("output_limit")))
        {
          for (dof_id_type i = 0;
               i <
               _sampler->parameters().get<std::vector<DistributionName>>("distributions").size();
               ++i)
            (*_inputs[i]) = _prev_val[i];
          (*_output[0]) = 0.0;
        }
        else
        {
          for (dof_id_type i = 0;
               i <
               _sampler->parameters().get<std::vector<DistributionName>>("distributions").size();
               ++i)
          {
            (*_inputs[i]) = _sampler->getNextLocalRow()[i];
            _prev_val[i] = (*_inputs[i]);
          }
          (*_output[0]) = 1.0;
          _prev_val_out = (*_output[0]);
        }
      }
      else
      {
        /* This is the sampling phase of the Adaptive Importance Sampling algorithm.
           Here, all proposed samples by the AIS.C sampler are accepted since the importance
           distribution traning phase is finished.*/
        for (dof_id_type i = 0;
             i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size();
             ++i)
          (*_inputs[i]) = _sampler->getNextLocalRow()[i];
        _prev_val_out = (_sampler->parameters().get<bool>("use_absolute_value"))
                            ? std::abs(*_output[0])
                            : (*_output[0]);
        if (_prev_val_out >= (_sampler->parameters().get<Real>("output_limit")))
          (*_output[0]) = 1.0;
        else
          (*_output[0]) = 0.0;
      }
    }
    _check_step = _step;
  }
}
