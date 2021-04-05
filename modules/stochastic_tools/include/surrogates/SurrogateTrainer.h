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

#include "Sampler.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

class TrainingDataBase;
template <typename T>
class TrainingData;

/**
 * This is the base trainer class whose main functionality is the API for declaring
 * model data. All trainer must at least derive from this. Unless a trainer needs
 * to perform its own loop through data, it is highly recommended to derive from
 * SurrogateTrainer.
 */
class SurrogateTrainerBase : public GeneralUserObject
{
public:
  static InputParameters validParams();
  SurrogateTrainerBase(const InputParameters & parameters);

  virtual void initialize() {}                         // not required, but available
  virtual void finalize() {}                           // not required, but available
  virtual void threadJoin(const UserObject &) final {} // GeneralUserObjects are not threaded

  /**
   * The name for training data stored within the MooseApp
   */
  const std::string & modelMetaDataName() const { return _model_meta_data_name; }

  ///@{
  /**
   * Declare model data for loading from file as well as restart
   */
  // MOOSEDOCS_BEGIN
  template <typename T>
  T & declareModelData(const std::string & data_name);

  template <typename T>
  T & declareModelData(const std::string & data_name, const T & value);
  // MOOSEDOCS_END
  ///@}

private:
  /// Name for the meta data associated with training
  const std::string _model_meta_data_name;

  /**
   * Internal function used by public declareModelData methods.
   */
  template <typename T>
  RestartableData<T> & declareModelDataHelper(const std::string & data_name);
};

template <typename T>
T &
SurrogateTrainerBase::declareModelData(const std::string & data_name)
{
  RestartableData<T> & data_ref = declareModelDataHelper<T>(data_name);
  return data_ref.set();
}

template <typename T>
T &
SurrogateTrainerBase::declareModelData(const std::string & data_name, const T & value)
{
  RestartableData<T> & data_ref = declareModelDataHelper<T>(data_name);
  data_ref.set() = value;
  return data_ref.set();
}

template <typename T>
RestartableData<T> &
SurrogateTrainerBase::declareModelDataHelper(const std::string & data_name)
{
  auto data_ptr = libmesh_make_unique<RestartableData<T>>(data_name, nullptr);
  RestartableDataValue & value =
      _app.registerRestartableData(data_name, std::move(data_ptr), 0, false, _model_meta_data_name);
  RestartableData<T> & data_ref = static_cast<RestartableData<T> &>(value);
  return data_ref;
}

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
class SurrogateTrainer : public SurrogateTrainerBase
{
public:
  static InputParameters validParams();
  SurrogateTrainer(const InputParameters & parameters);

  virtual void initialize() final;
  virtual void execute() final;
  virtual void finalize() final{};

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

  // TRAINING_DATA_END

  /// Sampler being used for training
  Sampler & _sampler;

  /// During training loop, this is the row index of the data
  dof_id_type _row;
  /// During training loop, this is the local row index of the data
  dof_id_type _local_row;

private:
  /*
   * Called at the beginning of execute() to make sure values are set properly
   */
  void checkIntegrity() const;

  /// Sampler data for the current row
  std::vector<Real> _row_data;

  /// Vector of reporter names and their corresponding values (to be filled by getTrainingData)
  std::unordered_map<ReporterName, std::shared_ptr<TrainingDataBase>> _training_data;
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
