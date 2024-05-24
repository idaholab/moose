//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "GeneralUserObject.h"
#include "LoadSurrogateDataAction.h"
#include "RestartableModelInterface.h"

#include "Sampler.h"
#include "StochasticToolsApp.h"
#include "SurrogateModelInterface.h"
#include "MooseRandom.h"

class TrainingDataBase;
template <typename T>
class TrainingData;

/**
 * This is the base trainer class whose main functionality is the API for declaring
 * model data. All trainer must at least derive from this. Unless a trainer needs
 * to perform its own loop through data, it is highly recommended to derive from
 * SurrogateTrainer.
 */
class SurrogateTrainerBase : public GeneralUserObject, public RestartableModelInterface
{
public:
  static InputParameters validParams();
  SurrogateTrainerBase(const InputParameters & parameters);

  virtual void initialize() {}                         // not required, but available
  virtual void finalize() {}                           // not required, but available
  virtual void threadJoin(const UserObject &) final {} // GeneralUserObjects are not threaded
};

/**
 * This is the main trainer base class. The main purpose is to avoid a lot of code
 * duplication from performing sampler loops and dealing with distributed data. There
 * three functions that derived trainer should override: preTrain, train, and postTrain.
 * Derived class should also use the getTrainingData functionality, which provides a
 * refernce to vector reporter data in its current state within the sampler loop.
 *
 * The idea behind this is to emulate the element loop behaiviour in other MOOSE objects.
 * For instance, in a kernel, the value of _u corresponds to the solution in an element.
 * Here data referenced with getTrainingData will correspond to the the value of the
 * data in a sampler row.
 */
class SurrogateTrainer : public SurrogateTrainerBase, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  SurrogateTrainer(const InputParameters & parameters);

  virtual void initialize() final;
  virtual void execute() final;
  virtual void finalize() final {}

protected:
  /*
   * Setup function called before sampler loop
   */
  virtual void preTrain() {}

  /*
   * Function needed to be overried, called during sampler loop
   */
  virtual void train() {}

  /*
   * Function called after sampler loop, used for mpi communication mainly
   */
  virtual void postTrain() {}

  // TRAINING_DATA_BEGIN

  /*
   * Get a reference to training data given a reporter name
   */
  template <typename T>
  const T & getTrainingData(const ReporterName & rname);

  /*
   * Get a reference to the sampler row data
   */
  const std::vector<Real> & getSamplerData() const { return _row_data; };

  /*
   * Get a reference to the predictor row data
   */
  const std::vector<Real> & getPredictorData() const { return _predictor_data; };

  /*
   * Get current sample size (this is recalculated to reflect the number of skipped samples)
   */
  unsigned int getCurrentSampleSize() const { return _current_sample_size; };

  /*
   * Get current local sample size (recalculated to reflect number of skipped samples)
   */
  unsigned int getLocalSampleSize() const { return _local_sample_size; };

  // TRAINING_DATA_END

  /*
   * Evaluate CV error using _cv_surrogate and appropriate predictor row.
   */
  virtual std::vector<Real> evaluateModelError(const SurrogateModel & surr);

  // TRAINING_DATA_MEMBERS
  ///@{
  /// Sampler being used for training
  Sampler & _sampler;
  /// During training loop, this is the row index of the data
  dof_id_type _row;
  /// During training loop, this is the local row index of the data
  dof_id_type _local_row;
  /// Response value
  const Real * _rval;
  /// Vector response value
  const std::vector<Real> * _rvecval;
  /// Predictor values from reporters
  std::vector<const Real *> _pvals;
  /// Columns from sampler for predictors
  std::vector<unsigned int> _pcols;
  /// Dimension of predictor data - either _sampler.getNumberOfCols() or _pvals.size() + _pcols.size().
  unsigned int _n_dims;
  /// The number of outputs
  unsigned int & _n_outputs;
  ///@}
  // TRAINING_DATA_MEMBERS_END

private:
  /*
   * Called at the beginning of execute() to make sure values are set properly
   */
  void checkIntegrity() const;

  /*
   * Main model training method - called during crossValidate() and for final model training.
   */
  void executeTraining();

  /*
   * Call if cross-validation is turned on.
   */
  std::vector<Real> crossValidate();

  /*
   * Update predictor row (uses both Sampler and Reporter values, according to _pvals and _pcols)
   */
  void updatePredictorRow();

  /// Sampler data for the current row
  std::vector<Real> _row_data;

  /// Predictor data for current row - can be combination of Sampler and Reporter values.
  std::vector<Real> _predictor_data;

  /// Whether or not we are skipping samples that have unconverged solutions
  const bool _skip_unconverged;

  /// Whether or not the current sample has a converged solution
  const bool * _converged;

  /// Number of samples used to train the model.
  unsigned int _current_sample_size;

  /// Number of samples (locally) used to train the model.
  unsigned int _local_sample_size;

  /// Vector of reporter names and their corresponding values (to be filled by getTrainingData)
  std::unordered_map<ReporterName, std::shared_ptr<TrainingDataBase>> _training_data;

  /*
   * Variables related to cross validation.
   */
  ///@{
  /// Vector of indices to skip during executeTraining()
  std::vector<dof_id_type> _skip_indices;
  /// Type of cross validation to perform - for now, just 'none' (no CV) or 'k_fold'
  const MooseEnum & _cv_type;
  /// Number of splits (k) to split sampler data into.
  const unsigned int & _n_splits;
  /// Number of repeated trials of cross validation to perform.
  const unsigned int & _cv_n_trials;
  /// Seed used for _cv_generator.
  const unsigned int & _cv_seed;
  /// Random number generator used for shuffling sampler rows during splitting.
  MooseRandom _cv_generator;
  /// SurrogateModel used to evaluate model error relative to test points.
  const SurrogateModel * _cv_surrogate;
  /// Set to true if cross validation is being performed, controls behavior in execute().
  const bool _doing_cv;
  /// RMSE scores from each CV trial - can be grabbed by VPP or Reporter.
  std::vector<std::vector<Real>> & _cv_trial_scores;
  ///@}
};

template <typename T>
const T &
SurrogateTrainer::getTrainingData(const ReporterName & rname)
{
  auto it = _training_data.find(rname);
  if (it != _training_data.end())
  {
    auto data = std::dynamic_pointer_cast<TrainingData<T>>(it->second);
    if (!data)
      mooseError("Reporter value ", rname, " already exists but is of different type.");
    return data->get();
  }
  else
  {
    const std::vector<T> & rval = getReporterValueByName<std::vector<T>>(rname);
    _training_data[rname] = std::make_shared<TrainingData<T>>(rval);
    return std::dynamic_pointer_cast<TrainingData<T>>(_training_data[rname])->get();
  }
}

class TrainingDataBase
{
public:
  TrainingDataBase() : _is_distributed(false) {}

  virtual ~TrainingDataBase() = default;

  virtual dof_id_type size() const = 0;
  virtual void setCurrentIndex(dof_id_type index) = 0;
  bool & isDistributed() { return _is_distributed; }

protected:
  bool _is_distributed;
};

template <typename T>
class TrainingData : public TrainingDataBase
{
public:
  TrainingData(const std::vector<T> & vector) : _vector(vector) {}

  virtual dof_id_type size() const override { return _vector.size(); }
  virtual void setCurrentIndex(dof_id_type index) override { _value = _vector[index]; }

  const T & get() const { return _value; }

private:
  const std::vector<T> & _vector;
  T _value;
};
