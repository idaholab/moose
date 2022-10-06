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
  MooseEnum learning_function("Ufunction COV");
  params.addClassDescription("Evaluates parsed function to determine if sample needs to be "
                             "evaluated, otherwise data is set to a default value.");
  params.addRequiredParam<MooseEnum>(
      "learning_function", learning_function, "The learning function for active learning.");
  params.addRequiredParam<Real>("learning_function_threshold", "The learning function threshold.");
  params.addParam<Real>("learning_function_parameter", "The learning function parameter.");
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
  _learning_function(getParam<MooseEnum>("learning_function")),
  _learning_function_threshold(getParam<Real>("learning_function_threshold")),
  _learning_function_parameter(isParamValid("learning_function_parameter") ? &getParam<Real>("learning_function_parameter") : nullptr),
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
  
  if (_learning_function == "Ufunction")
  {
    if (!isParamValid("learning_function_parameter"))
      ::mooseError("The Ufunction requires the model failure threshold to be specified.");
  }

}

bool
ActiveLearningGPDecision::learningFunction(const Real & gp_mean,
                                           const Real & gp_std,
                                           const MooseEnum & function_name,
                                           const Real & parameter,
                                           const Real & threshold)
{ 
  bool result = 0;
  Real f_val;
  if (function_name == "Ufunction")
  {
    f_val = std::abs(gp_mean-parameter) / gp_std;
    if (f_val > threshold)
      result = 1;
  }
  else if (function_name == "COV")
  {
    f_val = gp_std / std::abs(gp_mean);
    if (f_val < threshold)
      result = 1;
  }
  else
    ::mooseError("Invalid learning function ", std::string(function_name));
  return result;
}

bool
ActiveLearningGPDecision::needSample(const std::vector<Real> & row,
                                              dof_id_type local_ind,
                                              dof_id_type,
                                              Real & val)
{
  _gp_sto.resize(2);
  _output_comm.resize(1);
  _gp_mean_comm.resize(1);
  _gp_std_comm.resize(1);
  DenseMatrix<Real> inputs_comm(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  // Storing all the inputs and outputs (required for each time step of the mainApp)
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      inputs_comm(ss, j) = data[j];
  }
  _output_comm[local_ind] = val;
  _local_comm.sum(inputs_comm.get_values());
  _local_comm.allgather(_output_comm);
  if (_step <= _n_train) // Wait until all the training data is generated
  {
    if (_step > 2)
    {
      for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
      {
        _outputs_sto.push_back(_output_comm[ss]);
        for (unsigned int k = 0; k < _sampler.getNumberOfCols(); ++k)
          _inputs_sto[k].push_back(_inputs_prev[ss][k]);
      }
    }
    if (_step == _n_train) // Once training data is generated, train the GP
    {
      if (local_ind == 0)
        _al_gp->reTrain(_inputs_sto, _outputs_sto);
      
      // Setting up variables and making decisions
        // @{
      _gp_sto[0] = getSurrogateModel<GaussianProcess>("gp_evaluator").evaluate(row, _gp_sto[1]);
      val = _gp_sto[0];
      bool lf_indicator = learningFunction(_gp_mean[local_ind], _gp_std[local_ind], _learning_function, *_learning_function_parameter, _learning_function_threshold);
      if (lf_indicator)
      {
        val = _gp_sto[0];
        _decision[local_ind] = false;
      } else
      {
        ++_track_gp_fails;
        _flag_sample[local_ind] = true;
      }
      _gp_mean_comm[local_ind] = _gp_sto[0];
      _gp_std_comm[local_ind] = _flag_sample[0] == true ? 0.0 : _gp_sto[1];
      _local_comm.allgather(_gp_mean_comm);
      _local_comm.allgather(_gp_std_comm);
      for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
      {
        for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
          _inputs[ss][j] = inputs_comm(ss, j);
        _gp_mean[ss] = _gp_mean_comm[ss];
        _gp_std[ss] = _gp_std_comm[ss];
      }
        // @}
      // Finished setting up variables and making decisions

    }
    // Start tracking the GP failures until a user-specified batch size is met
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (unsigned int i = 0; i < _flag_sample.size(); ++i)
        _decision[i] = true;
    }
  } else // Training data generation and GP training completed. Active learning starts.
  {
    // If the number of GP fails greater than user-specified batch size, retrain GP
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
      {
        _outputs_sto.push_back(_output_comm[ss]);
        for (unsigned int k = 0; k < _sampler.getNumberOfCols(); ++k)
          _inputs_sto[k].push_back(_inputs_prev[ss][k]);
      }
      _al_gp->reTrain(_inputs_sto, _outputs_sto);
      _track_gp_fails = 0;
    }

    // Setting up variables and making decisions
      // @{
    _gp_sto[0] = getSurrogateModel<GaussianProcess>("gp_evaluator").evaluate(row, _gp_sto[1]);
    bool lf_indicator = learningFunction(_gp_mean[local_ind], _gp_std[local_ind], _learning_function, *_learning_function_parameter, _learning_function_threshold);
    if (_flag_sample[local_ind] == true)
      _flag_sample[local_ind] = false;
    if (lf_indicator)
    {
      val = _gp_sto[0];
      _decision[local_ind] = false;
    } else
    {
      ++_track_gp_fails;
      _flag_sample[local_ind] = true;
    }
    _gp_mean_comm[local_ind] = _gp_sto[0];
    _gp_std_comm[local_ind] = _flag_sample[0] == true ? 0.0 : _gp_sto[1];
    _local_comm.allgather(_gp_mean_comm);
    _local_comm.allgather(_gp_std_comm);
    for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
    {
      for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
        _inputs[ss][j] = inputs_comm(ss, j);
      _gp_mean[ss] = _gp_mean_comm[ss];
      _gp_std[ss] = _gp_std_comm[ss];
    }
      // @}
    // Finished setting up variables and making decisions

    // Start re-tracking the GP failures until a user-specified batch size is met
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (unsigned int i = 0; i < _flag_sample.size(); ++i)
        _decision[i] = true;
    }
  }
  // Storage of previous step inputs for usage in the next step
  for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
  {
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      _inputs_prev[ss][j] = inputs_comm(ss, j);
  }
  return _decision[local_ind];
}
