//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateTrainer.h"
#include "SurrogateModel.h"
#include "Sampler.h"
#include "StochasticToolsApp.h"
#include "MooseRandom.h"
#include "Shuffle.h"
#include <algorithm>

InputParameters
SurrogateTrainerBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += RestartableModelInterface::validParams();
  params.registerBase("SurrogateTrainer");
  return params;
}

SurrogateTrainerBase::SurrogateTrainerBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    RestartableModelInterface(*this, /*read_only=*/false, _type + "_" + name())
{
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

  // Common Training Data
  MooseEnum data_type("real=0 vector_real=1", "real");
  params.addRequiredParam<ReporterName>(
      "response",
      "Reporter value of response results, can be vpp with <vpp_name>/<vector_name> or sampler "
      "column with 'sampler/col_<index>'.");
  params.addParam<MooseEnum>("response_type", data_type, "Response data type.");
  params.addParam<std::vector<ReporterName>>(
      "predictors",
      std::vector<ReporterName>(),
      "Reporter values used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<std::vector<unsigned int>>(
      "predictor_cols",
      std::vector<unsigned int>(),
      "Sampler columns used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  // End Common Training Data

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
    _rval(nullptr),
    _rvecval(nullptr),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _n_outputs(declareModelData<unsigned int>("_n_outputs", 1)),
    _row_data(_sampler.getNumberOfCols()),
    _skip_unconverged(getParam<bool>("skip_unconverged_samples")),
    _converged(nullptr),
    _cv_type(getParam<MooseEnum>("cv_type")),
    _n_splits(getParam<unsigned int>("cv_splits")),
    _cv_n_trials(getParam<unsigned int>("cv_n_trials")),
    _cv_seed(getParam<unsigned int>("cv_seed")),
    _doing_cv(_cv_type != "none"),
    _cv_trial_scores(declareModelData<std::vector<std::vector<Real>>>("cv_scores"))
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

    _cv_generator.seed(0, _cv_seed);
  }

  // Get TrainingData for responses and predictors
  if (getParam<MooseEnum>("response_type") == 0)
    _rval = &getTrainingData<Real>(getParam<ReporterName>("response"));
  else if (getParam<MooseEnum>("response_type") == 1)
    _rvecval = &getTrainingData<std::vector<Real>>(getParam<ReporterName>("response"));

  const auto & pnames = getParam<std::vector<ReporterName>>("predictors");
  for (unsigned int i = 0; i < pnames.size(); ++i)
    _pvals[i] = &getTrainingData<Real>(pnames[i]);

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_pvals.empty() && _pcols.empty())
  {
    _pcols.resize(_sampler.getNumberOfCols());
    std::iota(_pcols.begin(), _pcols.end(), 0);
  }
  _n_dims = _pvals.size() + _pcols.size();

  _predictor_data.resize(_n_dims);
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
    {
      std::vector<Real> trial_score = crossValidate();

      // Expand _cv_trial_scores with more columns if necessary, then insert values.
      for (unsigned int r = _cv_trial_scores.size(); r < trial_score.size(); ++r)
        _cv_trial_scores.push_back(std::vector<Real>(_cv_n_trials, 0.0));
      for (auto r : make_range(trial_score.size()))
        _cv_trial_scores[r][trial] = trial_score[r];
    }

  _current_sample_size = _sampler.getNumberOfRows();
  _local_sample_size = _sampler.getNumberOfLocalRows();
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

    updatePredictorRow();

    if ((!_skip_unconverged || *_converged) &&
        std::find(_skip_indices.begin(), _skip_indices.end(), _row) == _skip_indices.end())
      train();

    _local_row++;
  }

  postTrain();
}

std::vector<Real>
SurrogateTrainer::crossValidate()
{
  std::vector<Real> cv_score(1, 0.0);

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

    _current_sample_size = _sampler.getNumberOfRows() - split_ids_buffer.size();

    auto first = std::lower_bound(
        split_ids_buffer.begin(), split_ids_buffer.end(), _sampler.getLocalRowBegin());
    auto last = std::upper_bound(
        split_ids_buffer.begin(), split_ids_buffer.end(), _sampler.getLocalRowEnd());
    _skip_indices.insert(_skip_indices.begin(), first, last);

    _local_sample_size = _sampler.getNumberOfLocalRows() - _skip_indices.size();

    // Train the model
    executeTraining();

    // Evaluate the model
    std::vector<Real> split_mse(1, 0.0);
    std::vector<Real> row_mse(1, 0.0);

    auto skipped_row = _skip_indices.begin();

    for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
    {
      const std::vector<Real> row = _sampler.getNextLocalRow();
      if (skipped_row != _skip_indices.end() && p == *skipped_row)
      {
        for (unsigned int i = 0; i < _row_data.size(); ++i)
          _row_data[i] = row[i];

        for (auto & pair : _training_data)
          pair.second->setCurrentIndex(
              (pair.second->isDistributed() ? p - _sampler.getLocalRowBegin() : p));

        updatePredictorRow();

        row_mse = evaluateModelError(*_cv_surrogate);

        // Expand split_mse if needed.
        split_mse.resize(row_mse.size(), 0.0);

        // Increment errors
        for (unsigned int r = 0; r < split_mse.size(); ++r)
          split_mse[r] += row_mse[r];

        skipped_row++;
      }
    }
    gatherSum(split_mse);

    // Expand cv_score if necessary.
    cv_score.resize(split_mse.size(), 0.0);

    for (auto r : make_range(split_mse.size()))
      cv_score[r] += split_mse[r] / n_rows;

    _skip_indices.clear();
  }

  for (auto r : make_range(cv_score.size()))
    cv_score[r] = std::sqrt(cv_score[r]);

  return cv_score;
}

std::vector<Real>
SurrogateTrainer::evaluateModelError(const SurrogateModel & surr)
{
  std::vector<Real> error(1, 0.0);

  if (_rval)
  {
    Real model_eval = surr.evaluate(_predictor_data);
    error[0] = MathUtils::pow(model_eval - (*_rval), 2);
  }
  else if (_rvecval)
  {
    error.resize(_rvecval->size());

    // Evaluate for vector response.
    std::vector<Real> model_eval(error.size());
    surr.evaluate(_predictor_data, model_eval);
    for (auto r : make_range(_rvecval->size()))
      error[r] = MathUtils::pow(model_eval[r] - (*_rvecval)[r], 2);
  }

  return error;
}

void
SurrogateTrainer::updatePredictorRow()
{
  unsigned int d = 0;
  for (const auto & val : _pvals)
    _predictor_data[d++] = *val;
  for (const auto & col : _pcols)
    _predictor_data[d++] = _row_data[col];
}
