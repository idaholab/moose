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
#include "SamplerInterface.h"
#include "LoadSurrogateDataAction.h"

class SurrogateModel : public GeneralUserObject, public SamplerInterface
{
public:
  static InputParameters validParams();

  SurrogateModel(const InputParameters & parameters);

  /**
   * Evaluate surrogate model given a row of parameters.
   */
  virtual Real evaluate(const std::vector<Real> & x) const = 0;

  /**
   * Train the surrogate model.
   */
  virtual void train() = 0;

  ///@{
  /**
   * Initialize/Finalize that only execute when training.
   */
  virtual void trainInitialize() {}
  virtual void trainFinalize() {}
  ///@}

  /**
   * Return True if the object is performing training (i.e., filename is not set)
   */
  bool isTraining() const;

  ///@{
  /**
   * Declare model data for loading from file as well as restart
   */
  template <typename T>
  T & declareModelData(const std::string & data_name);

  template <typename T>
  T & declareModelData(const std::string & data_name, const T & value);
  ///@}

  /**
   * UserObject methods that are used to implement the training/evaluate interfaces.
   */
  virtual void initialize() final;
  virtual void execute() final;
  virtual void finalize() final;
  virtual void threadJoin(const UserObject &) final {} // GeneralUserObjects are not threaded

private:
  /**
   * Internal function used by public declareModelData methods.
   */
  template <typename T>
  RestartableData<T> & declareModelDataHelper(const std::string & data_name);

  /// see isTraining
  const bool _is_training;
};

template <typename T>
T &
SurrogateModel::declareModelData(const std::string & data_name)
{
  RestartableData<T> & data_ref = declareModelDataHelper<T>(data_name);
  return data_ref.get();
}

template <typename T>
T &
SurrogateModel::declareModelData(const std::string & data_name, const T & value)
{
  RestartableData<T> & data_ref = declareModelDataHelper<T>(data_name);
  data_ref.set() = value;
  return data_ref.get();
}

template <typename T>
RestartableData<T> &
SurrogateModel::declareModelDataHelper(const std::string & data_name)
{
  auto data_ptr = libmesh_make_unique<RestartableData<T>>(data_name, nullptr);
  RestartableDataValue & value =
      _app.registerRestartableData(data_name, std::move(data_ptr), 0, false, name());
  RestartableData<T> & data_ref = static_cast<RestartableData<T> &>(value);
  return data_ref;
}
