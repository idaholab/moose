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
#include "MooseObject.h"
#include "SamplerInterface.h"
#include "SurrogateModelInterface.h"

class SurrogateModel : public MooseObject, public SamplerInterface, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  SurrogateModel(const InputParameters & parameters);

  /**
   * Evaluate surrogate model given a row of parameters.
   */
  virtual Real evaluate(const std::vector<Real> & x) const = 0;

  /**
   * The name for training data stored within the MooseApp
   */
  const std::string & modelMetaDataName() const { return _model_meta_data_name; }

  ///@{
  /**
   * Declare model data for loading from file as well as restart
   */
  template <typename T>
  const T & getModelData(const std::string & data_name) const;
  ///@}

private:
  /// Name used for model data. If a SurrogateTrainer object is supplied it's name is used. This
  /// results in the SurrogateModel having a reference to the training data so it is always current
  const std::string _model_meta_data_name;

  /**
   * Internal function used by public declareModelData methods.
   */
  template <typename T>
  RestartableData<T> & getModelDataHelper(const std::string & data_name) const;
};

template <typename T>
const T &
SurrogateModel::getModelData(const std::string & data_name) const
{
  RestartableData<T> & data_ref = getModelDataHelper<T>(data_name);
  return data_ref.get();
}

template <typename T>
RestartableData<T> &
SurrogateModel::getModelDataHelper(const std::string & data_name) const
{
  auto data_ptr = libmesh_make_unique<RestartableData<T>>(data_name, nullptr);
  RestartableDataValue & value =
      _app.registerRestartableData(data_name, std::move(data_ptr), 0, true, _model_meta_data_name);
  RestartableData<T> & data_ref = static_cast<RestartableData<T> &>(value);
  return data_ref;
}
