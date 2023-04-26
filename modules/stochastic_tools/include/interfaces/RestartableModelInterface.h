//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Restartable.h"
#include "StochasticToolsApp.h"

/**
 * An interface class which manages the model data save and load functionalities from moose objects
 * (such as surrogates, mappings, etc.) in the stochastic tools module.
 * */
class RestartableModelInterface
{
public:
  static InputParameters validParams();

  RestartableModelInterface(const MooseObject * object,
                            const bool read_only,
                            const std::string & meta_data_name);

  ///@{
  /**
   * Declare model data for loading from file as well as restart
   */
  // MOOSEDOCS_BEGIN
  template <typename T, typename... Args>
  T & declareModelData(const std::string & data_name, Args &&... args);
  // MOOSEDOCS_END
  ///@}

  ///@{
  /**
   * Retrieve model data from the interface
   */
  template <typename T, typename... Args>
  const T & getModelData(const std::string & data_name, Args &&... args) const;
  ///@}

  const std::string & modelMetaDataName() const { return _model_meta_data_name; }

  const std::string _model_meta_data_name;

private:
  /// Reference to FEProblemBase instance
  const MooseObject * _object;

  Restartable _restartable;
};

template <typename T, typename... Args>
T &
RestartableModelInterface::declareModelData(const std::string & data_name, Args &&... args)
{
  return _restartable.declareRestartableData<T>(data_name, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
const T &
RestartableModelInterface::getModelData(const std::string & data_name, Args &&... args) const
{
  return _restartable.getRestartableData<T>(data_name, std::forward<Args>(args)...);
}
