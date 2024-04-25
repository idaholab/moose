//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "BayesianGPryLearner.h"
#include "DenseMatrix.h"
#include "AdaptiveMonteCarloUtils.h"
#include "StochasticToolsUtils.h"

registerMooseObject("StochasticToolsApp", BayesianGPryLearner);

InputParameters
BayesianGPryLearner::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Fast Bayesian inference with the GPry algorithm by El Gammal et al. "
                             "2023: NN and GP training step.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "output_comm", "output_comm", "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("al_nn", "Active learning NN trainer.");
  return params;
}

BayesianGPryLearner::BayesianGPryLearner(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_comm(declareValue<std::vector<Real>>("output_comm")),
    _sampler(getSampler("sampler")),
    _gpry_sampler(dynamic_cast<const BayesianGPrySampler *>(&_sampler)),
    _al_nn(getUserObject<ActiveLearningLibtorchNN>("al_nn")),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm())
{
  // Check whether the selected sampler is an adaptive sampler or not
  if (!_gpry_sampler)
    paramError("sampler", "The selected sampler is not of type BayesianGPrySampler.");
}

void
BayesianGPryLearner::setupNNData(const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  tmp.resize(_sampler.getNumberOfCols());
  for (unsigned int i = 0; i < _output_comm.size(); ++i)
  {
    if (_output_comm[i] > 0.0)
      _nn_outputs.push_back(1.0);
    else
      _nn_outputs.push_back(0.0);
    for (unsigned int j = 0; j < tmp.size(); ++j)
      tmp[j] = data_in(i, j);
    _nn_inputs.push_back(tmp);
  }
}

void
BayesianGPryLearner::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }

  DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  _local_comm.sum(data_in.get_values());
  _output_comm = _output_value;
//   std::cout << "_output_value " << Moose::stringify(_output_value) << std::endl;
  _local_comm.allgather(_output_comm);
//   std::cout << "_output_comm " << Moose::stringify(_output_comm) << std::endl;

  setupNNData(data_in);

  bool read_nn = false; //  = _check_step > 1 ? false : true;
//   if (_t_step > 1)
//     read_nn = true;

  _al_nn.reTrain(_nn_inputs, _nn_outputs, read_nn);

  // Track the current step
  _check_step = _t_step;
}

#endif
