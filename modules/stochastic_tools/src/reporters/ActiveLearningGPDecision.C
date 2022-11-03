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

ActiveLearningGPDecision::ActiveLearningGPDecision(const InputParameters & parameters)
  : ActiveLearningReporterTempl<Real>(parameters),
    SurrogateModelInterface(this),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _learning_function(getParam<MooseEnum>("learning_function")),
    _learning_function_threshold(getParam<Real>("learning_function_threshold")),
    _learning_function_parameter(isParamValid("learning_function_parameter")
                                     ? &getParam<Real>("learning_function_parameter")
                                     : nullptr),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcess>("gp_evaluator")),
    _sampler(getSampler("sampler")),
    _flag_sample(declareValue<std::vector<bool>>("flag_sample")),
    _n_train(getParam<int>("n_train")),
    _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _gp_mean(declareValue<std::vector<Real>>("gp_mean")),
    _gp_std(declareValue<std::vector<Real>>("gp_std"))
{
  _inputs_batch.resize(_sampler.getNumberOfCols());
  _flag_sample.resize(_sampler.getNumberOfRows(), false);
  _decision.resize(_sampler.getNumberOfRows(), true);
  _inputs.resize(_sampler.getNumberOfRows(), std::vector<Real>(_sampler.getNumberOfCols()));
  _inputs_batch_prev.resize(_sampler.getNumberOfRows(),
                            std::vector<Real>(_sampler.getNumberOfCols()));
  _gp_mean.resize(_sampler.getNumberOfRows());
  _gp_std.resize(_sampler.getNumberOfRows());
  _track_gp_fails = 0;
  _allowed_gp_fails = _sampler.getNumberOfRows();
  _communicator.split(
      _sampler.getNumberOfLocalRows() > 0 ? 1 : MPI_UNDEFINED, processor_id(), _local_comm);

  if (_learning_function == "Ufunction" && !isParamValid("learning_function_parameter"))
    paramError("learning_function_parameter",
               "The Ufunction requires the model failure threshold to be specified.");
}

bool
ActiveLearningGPDecision::learningFunction(const Real & gp_mean,
                                           const Real & gp_std,
                                           const MooseEnum & function_name,
                                           const Real & parameter,
                                           const Real & threshold)
{
  if (function_name == "Ufunction")
    return (std::abs(gp_mean - parameter) / gp_std) > threshold;
  else if (function_name == "COV")
    return (gp_std / std::abs(gp_mean)) < threshold;
  else
    mooseError("Invalid learning function ", std::string(function_name));
  return false;
}

void
ActiveLearningGPDecision::setupData(const std::vector<Real> & output_parallel,
                                    const std::vector<std::vector<Real>> & inputs_prev)
{
  _outputs_batch.insert(_outputs_batch.end(), output_parallel.begin(), output_parallel.end());
  for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols(); ++k)
      _inputs_batch[k].push_back(inputs_prev[ss][k]);
  }
}

void
ActiveLearningGPDecision::facilitateDecision(const std::vector<Real> & row,
                                             dof_id_type local_ind,
                                             Real & val,
                                             const bool & retrain)
{
  _gp_sto[0] = _gp_eval.evaluate(row, _gp_sto[1]);
  _gp_sto[1] = retrain ? 0.0 : _gp_sto[1];
  const bool lf_indicator = learningFunction(_gp_sto[0],
                                             _gp_sto[1],
                                             _learning_function,
                                             *_learning_function_parameter,
                                             _learning_function_threshold);
  _flag_sample[local_ind] = false;
  if (lf_indicator)
  {
    val = _gp_sto[0];
    _decision[local_ind] = false;
  }
  else
  {
    ++_track_gp_fails;
    _flag_sample[local_ind] = true;
  }
  _gp_mean_parallel[local_ind] = _gp_sto[0];
  _gp_std_parallel[local_ind] = _flag_sample[local_ind] == true ? 0.0 : _gp_sto[1];
}

void
ActiveLearningGPDecision::transferOutput(const DenseMatrix<Real> & inputs_parallel,
                                         const std::vector<Real> & gp_mean_parallel,
                                         const std::vector<Real> & gp_std_parallel)
{
  for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      _inputs[ss][j] = inputs_parallel(ss, j);
  _gp_mean = gp_mean_parallel;
  _gp_std = gp_std_parallel;
}

bool
ActiveLearningGPDecision::needSample(const std::vector<Real> & row,
                                     dof_id_type local_ind,
                                     dof_id_type,
                                     Real & val)
{
  _gp_sto.resize(2);
  _output_parallel.resize(1);
  _gp_mean_parallel.resize(1);
  _gp_std_parallel.resize(1);
  DenseMatrix<Real> inputs_parallel(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  // Storing all the inputs and outputs (required for each time step of the mainApp)
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      inputs_parallel(ss, j) = data[j];
  }
  _output_parallel[local_ind] = val;
  _local_comm.sum(inputs_parallel.get_values());
  _local_comm.allgather(_output_parallel);
  if (_step <= _n_train) // Wait until all the training data is generated
  {
    if (_step > 2)
      setupData(_output_parallel, _inputs_batch_prev);
    if (_step == _n_train) // Once training data is generated, train the GP
    {
      if (local_ind == 0)
        _al_gp.reTrain(_inputs_batch, _outputs_batch);
      // Setting up variables and making decisions
      facilitateDecision(row, local_ind, val, true);
      _local_comm.allgather(_gp_mean_parallel);
      _local_comm.allgather(_gp_std_parallel);
      transferOutput(inputs_parallel, _gp_mean_parallel, _gp_std_parallel);
    }
    // Start tracking the GP failures until a user-specified batch size is met
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (unsigned int i = 0; i < _flag_sample.size(); ++i)
        _decision[i] = true;
    }
  }
  else // Training data generation and GP training completed. Active learning starts.
  {
    bool retrain = _track_gp_fails >= _allowed_gp_fails;
    // If the number of GP fails is greater than the user-specified batch size, retrain GP
    if (retrain)
    {
      setupData(_output_parallel, _inputs_batch_prev);
      _al_gp.reTrain(_inputs_batch, _outputs_batch);
      _track_gp_fails = 0;
    }
    // Setting up variables and making decisions
    facilitateDecision(row, local_ind, val, retrain);
    _local_comm.allgather(_gp_mean_parallel);
    _local_comm.allgather(_gp_std_parallel);
    transferOutput(inputs_parallel, _gp_mean_parallel, _gp_std_parallel);
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
      _inputs_batch_prev[ss][j] = inputs_parallel(ss, j);
  }
  return _decision[local_ind];
}
