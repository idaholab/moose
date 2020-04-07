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

class SurrogateTrainer : public GeneralUserObject, public SamplerInterface
{
public:
  static InputParameters validParams();
  SurrogateTrainer(const InputParameters & parameters);
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
SurrogateTrainer::declareModelData(const std::string & data_name)
{
  RestartableData<T> & data_ref = declareModelDataHelper<T>(data_name);
  return data_ref.get();
}

template <typename T>
T &
SurrogateTrainer::declareModelData(const std::string & data_name, const T & value)
{
  RestartableData<T> & data_ref = declareModelDataHelper<T>(data_name);
  data_ref.set() = value;
  return data_ref.get();
}

template <typename T>
RestartableData<T> &
SurrogateTrainer::declareModelDataHelper(const std::string & data_name)
{
  auto data_ptr = libmesh_make_unique<RestartableData<T>>(data_name, nullptr);
  RestartableDataValue & value =
      _app.registerRestartableData(data_name, std::move(data_ptr), 0, false, _model_meta_data_name);
  RestartableData<T> & data_ref = static_cast<RestartableData<T> &>(value);
  return data_ref;
}
