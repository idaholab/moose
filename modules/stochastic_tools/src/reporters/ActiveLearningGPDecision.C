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
  params.addClassDescription("Evaluates parsed function to determine if sample needs to be "
                             "evaluated, otherwise data is set to a default value.");
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

ActiveLearningGPDecision::ActiveLearningGPDecision(
    const InputParameters & parameters)
  : ActiveLearningReporterTempl<Real>(parameters),
  SurrogateModelInterface(this),
  _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
  _al_gp(&getUserObject<ActiveLearningGaussianProcess>("al_gp")),
  _sampler(getSampler("sampler")),
  _flag_sample(declareValue<std::vector<bool>>("flag_sample")),
  _n_train(getParam<int>("n_train")),
  _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
  _gp_mean(declareValue<std::vector<Real>>("gp_mean")),
  _gp_std(declareValue<std::vector<Real>>("gp_std"))
{
  _inputs_sto.resize(_sampler.getNumberOfCols());
  _flag_sample.resize(_sampler.getNumberOfRows());
  _decision.resize(_sampler.getNumberOfRows());
  for (unsigned int i = 0; i < _flag_sample.size(); ++i)
  {
    _flag_sample[i] = false;
    _decision[i] = true;
  }
  _inputs.resize(_sampler.getNumberOfRows());
  _inputs_prev.resize(_sampler.getNumberOfRows());
  for (unsigned int i = 0; i < _sampler.getNumberOfRows(); ++i)
  {
    _inputs[i].resize(_sampler.getNumberOfCols());
    _inputs_prev[i].resize(_sampler.getNumberOfCols());
  }
  _gp_mean.resize(_sampler.getNumberOfRows());
  _gp_std.resize(_sampler.getNumberOfRows());
  _track_gp_fails = 0;
  _allowed_gp_fails = _sampler.getNumberOfRows();
  _communicator.split(
      _sampler.getNumberOfLocalRows() > 0 ? 1 : MPI_UNDEFINED, processor_id(), _local_comm);
}

bool
ActiveLearningGPDecision::needSample(const std::vector<Real> & row,
                                              dof_id_type local_ind,
                                              dof_id_type,
                                              Real & val)
{
  std::vector<Real> result;
  result.resize(2);
  std::vector<Real> output1;
  std::vector<Real> gp_mean1;
  std::vector<Real> gp_std1;
  output1.resize(1);
  gp_mean1.resize(1);
  gp_std1.resize(1);
  DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  output1[local_ind] = val;
  _local_comm.sum(data_in.get_values());
  _local_comm.allgather(output1);
  if (_step <= _n_train)
  {
    if (_step > 2)
    {
      for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
      {
        _outputs_sto.push_back(output1[ss]);
        for (unsigned int k = 0; k < _sampler.getNumberOfCols(); ++k)
          _inputs_sto[k].push_back(_inputs_prev[ss][k]);
      }
    }
    if (_step == _n_train)
    {
      if (local_ind == 0)
        _al_gp->reTrain(_inputs_sto, _outputs_sto);
      result[0] = getSurrogateModel<GaussianProcess>("gp_evaluator").evaluate(row, result[1]);
      val = result[0];
      Real U_val;
      U_val = std::abs(_gp_mean[local_ind]-349.345)/_gp_std[local_ind]; // result[1] / std::abs(result[0]); //
      if (U_val > 2.0) // < 0.025 // 
      {
        val = result[0];
        _decision[local_ind] = false;
      } else
      {
        ++_track_gp_fails;
        _flag_sample[local_ind] = true;
      }
      gp_mean1[local_ind] = result[0];
      gp_std1[local_ind] = _flag_sample[0] == true ? 0.0 : result[1];
      _local_comm.allgather(gp_mean1);
      _local_comm.allgather(gp_std1);
      for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
      {
        for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
          _inputs[ss][j] = data_in(ss, j);
        _gp_mean[ss] = gp_mean1[ss];
        _gp_std[ss] = gp_std1[ss];
      }
    }
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (unsigned int i = 0; i < _flag_sample.size(); ++i)
        _decision[i] = true;
    }
  } else
  {
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
      {
        _outputs_sto.push_back(output1[ss]);
        for (unsigned int k = 0; k < _sampler.getNumberOfCols(); ++k)
          _inputs_sto[k].push_back(_inputs_prev[ss][k]);
      }
      _al_gp->reTrain(_inputs_sto, _outputs_sto);
      _track_gp_fails = 0;
    }
    result[0] = getSurrogateModel<GaussianProcess>("gp_evaluator").evaluate(row, result[1]);
    Real U_val;
    U_val = std::abs(_gp_mean[local_ind]-349.345)/_gp_std[local_ind]; // result[1] / std::abs(result[0]); //
    if (_flag_sample[local_ind] == true)
      _flag_sample[local_ind] = false;
    if (U_val > 2.0) // < 0.025 // 
    {
      val = result[0];
      _decision[local_ind] = false;
    } else
    {
      ++_track_gp_fails;
      _flag_sample[local_ind] = true;
    }
    gp_mean1[local_ind] = result[0];
    gp_std1[local_ind] = _flag_sample[0] == true ? 0.0 : result[1];
    _local_comm.allgather(gp_mean1);
    _local_comm.allgather(gp_std1);
    for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
    {
      for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
        _inputs[ss][j] = data_in(ss, j);
      _gp_mean[ss] = gp_mean1[ss];
      _gp_std[ss] = gp_std1[ss];
    }
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (unsigned int i = 0; i < _flag_sample.size(); ++i)
        _decision[i] = true;
    }
  }
  for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
  {
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      _inputs_prev[ss][j] = data_in(ss, j);
  }
  return _decision[local_ind];
}