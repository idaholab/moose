//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateTrainer.h"
#include "Sampler.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"
#include "MooseRandom.h"
#include "Shuffle.h"
#include <algorithm>

InputParameters
SurrogateTrainerBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.registerBase("SurrogateTrainer");
  return params;
}

SurrogateTrainerBase::SurrogateTrainerBase(const InputParameters & parameters)
  : GeneralUserObject(parameters), _model_meta_data_name(_type + "_" + name())
{
  _app.registerRestartableDataMapName(_model_meta_data_name, name());
}

InputParameters
SurrogateTrainer::validParams()
{
  InputParameters params = SurrogateTrainerBase::validParams();
  params.addRequiredParam<SamplerName>("sampler",
                                       "Sampler used to create predictor and response data.");
  params.addParam<ReporterName>(
      "converged_reporter",
      "Reporter value used to determine if a sample's multiapp solve converged.");
  params.addParam<bool>("skip_unconverged_samples",
                        false,
                        "True to skip samples where the multiapp did not converge, "
                        "'stochastic_reporter' is required to do this.");

  MooseEnum cv_type("none=0 k_fold=1", "none");
  params.addParam<MooseEnum>(
      "cv_type",
      cv_type,
      "Cross-validation method to use for dataset. Options are 'none' or 'k_fold'.");
  params.addRangeCheckedParam<unsigned int>(
      "cv_splits", 10, "cv_splits > 1", "Number of splits (k) to use in k-fold cross-validation.");
  params.addParam<UserObjectName>("cv_surrogate",
                                  "Name of Surrogate object used for model cross-validation.");
  params.addParam<unsigned int>(
      "cv_n_trials", 1, "Number of repeated trials of cross-validation to perform.");
  params.addParam<unsigned int>("cv_seed",
                                std::numeric_limits<unsigned int>::max(),
                                "Seed used to initialize random number generator for data "
                                "splitting during cross validation.");

  return params;
}

SurrogateTrainer::SurrogateTrainer(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    SurrogateModelInterface(this),
    _sampler(getSampler("sampler")),
    _row_data(_sampler.getNumberOfCols()),
    _skip_unconverged(getParam<bool>("skip_unconverged_samples")),
    _converged(nullptr),
    _cv_type(getParam<MooseEnum>("cv_type")),
    _n_splits(getParam<unsigned int>("cv_splits")),
    _cv_n_trials(getParam<unsigned int>("cv_n_trials")),
    _cv_seed(getParam<unsigned int>("cv_seed")),
    _doing_cv(_cv_type != "none"),
    _cv_trial_scores(declareModelData<std::vector<Real>>("cv_scores"))
{
  if (_skip_unconverged)
  {
    if (!isParamValid("converged_reporter"))
      paramError("skip_unconverged_samples",
                 "'converged_reporter' needs to be specified to skip unconverged sample.");
    _converged = &getTrainingData<bool>(getParam<ReporterName>("converged_reporter"));
  }

  if (_doing_cv)
  {
    if (!isParamValid("cv_surrogate"))
      paramError("cv_type",
                 "To perform cross-validation, the option cv_surrogate needs to be specified",
                 " to provide a Surrogate object for training and evaluation.");

    if (_n_splits > _sampler.getNumberOfRows())
      paramError("cv_splits",
                 "The specified number of splits (cv_splits = ",
                 _n_splits,
                 ")",
                 " exceeds the number of rows in Sampler '",
                 getParam<SamplerName>("sampler"),
                 "'");

    if (type() != "PolynomialRegressionTrainer")
      paramWarning("cv_type",
                   "Cross validation has only been verified for PolynomialRegressionTrainer");

    _cv_trial_scores.resize(_cv_n_trials);
    _cv_generator.seed(0, _cv_seed);
  }
}

void
SurrogateTrainer::initialize()
{
  // Figure out if data is distributed
  for (auto & pair : _training_data)
  {
    const ReporterName & name = pair.first;
    TrainingDataBase & data = *pair.second;

    const auto & mode = _fe_problem.getReporterData().getReporterMode(name);
    if (mode == REPORTER_MODE_DISTRIBUTED || (mode == REPORTER_MODE_ROOT && processor_id() != 0))
      data.isDistributed() = true;
    else if (mode == REPORTER_MODE_REPLICATED ||
             (mode == REPORTER_MODE_ROOT && processor_id() == 0))
      data.isDistributed() = false;
    else
      mooseError("Predictor reporter value ", name, " is not of supported mode.");
  }

  if (_doing_cv)
    _cv_surrogate = &getSurrogateModel("cv_surrogate");
}

void
SurrogateTrainer::execute()
{
  if (_doing_cv)
    for (const auto & trial : make_range(_cv_n_trials))
      _cv_trial_scores[trial] = crossValidate();

  executeTraining();
}

void
SurrogateTrainer::checkIntegrity() const
{
  // Check that the number of sampler columns hasn't changed
  if (_row_data.size() != _sampler.getNumberOfCols())
    mooseError("Number of sampler columns has changed.");

  // Check that training data is correctly sized
  for (auto & pair : _training_data)
  {
    dof_id_type rsize = pair.second->size();
    dof_id_type nrow =
        pair.second->isDistributed() ? _sampler.getNumberOfLocalRows() : _sampler.getNumberOfRows();
    if (rsize != nrow)
      mooseError("Reporter value ",
                 pair.first,
                 " of size ",
                 rsize,
                 " does not match sampler size (",
                 nrow,
                 ").");
  }
}

void
SurrogateTrainer::executeTraining()
{
  checkIntegrity();

  _row = _sampler.getLocalRowBegin();
  _local_row = 0;

  preTrain();

  for (_row = _sampler.getLocalRowBegin(); _row < _sampler.getLocalRowEnd(); ++_row)
  {
    // Need to do this manually in order to keep the iterators valid
    const std::vector<Real> data = _sampler.getNextLocalRow();
    for (unsigned int i = 0; i < _row_data.size(); ++i)
      _row_data[i] = data[i];

    // Set training data
    for (auto & pair : _training_data)
      pair.second->setCurrentIndex((pair.second->isDistributed() ? _local_row : _row));

    if ((!_skip_unconverged || *_converged) &&
        std::find(_skip_indices.begin(), _skip_indices.end(), _row) == _skip_indices.end())
      train();

    _local_row++;
  }

  postTrain();
}

Real
SurrogateTrainer::crossValidate()
{
  Real cv_score = 0;

  // Get reference to response data
  // FIXME: assuming response data is from a single value from training data
  const auto & rval =
      std::dynamic_pointer_cast<TrainingData<Real>>(_training_data.begin()->second)->get();

  // Get skipped indices for each split
  dof_id_type n_rows = _sampler.getNumberOfRows();
  std::vector<std::vector<dof_id_type>> split_indices;
  if (processor_id() == 0)
  {
    std::vector<dof_id_type> indices_flat(n_rows);
    std::iota(indices_flat.begin(), indices_flat.end(), 0);
    MooseUtils::shuffle(indices_flat, _cv_generator, 0);

    split_indices.resize(_n_splits);
    for (const auto & k : make_range(_n_splits))
    {
      const dof_id_type num_ind = n_rows / _n_splits + (k < (n_rows % _n_splits) ? 1 : 0);
      split_indices[k].insert(split_indices[k].begin(),
                              std::make_move_iterator(indices_flat.begin()),
                              std::make_move_iterator(indices_flat.begin() + num_ind));
      std::sort(split_indices[k].begin(), split_indices[k].end());
      indices_flat.erase(indices_flat.begin(), indices_flat.begin() + num_ind);
    }
  }

  std::vector<dof_id_type> split_ids_buffer;
  for (const auto & k : make_range(_n_splits))
  {
    if (processor_id() == 0)
      split_ids_buffer = split_indices[k];
    _communicator.broadcast(split_ids_buffer, 0);

    auto first = std::lower_bound(
        split_ids_buffer.begin(), split_ids_buffer.end(), _sampler.getLocalRowBegin());
    auto last = std::upper_bound(
        split_ids_buffer.begin(), split_ids_buffer.end(), _sampler.getLocalRowEnd());
    _skip_indices.insert(_skip_indices.begin(), first, last);

    // Train the model
    executeTraining();

    // Evaluate the model
    Real split_mse = 0;
    auto skipped_row = _skip_indices.begin();
    for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
    {
      const std::vector<Real> row = _sampler.getNextLocalRow();
      if (p == *skipped_row)
      {
        for (auto & pair : _training_data)
          pair.second->setCurrentIndex(
              (pair.second->isDistributed() ? p - _sampler.getLocalRowBegin() : p));
        // FIXME: assuming the the only predictor data is from sampler rows
        Real model_eval = _cv_surrogate->evaluate(row);
        split_mse += MathUtils::pow(model_eval - rval, 2);
        skipped_row++;
      }
    }

    gatherSum(split_mse);
    cv_score += split_mse / n_rows;

    _skip_indices.clear();
  }

  return std::sqrt(cv_score);
}
