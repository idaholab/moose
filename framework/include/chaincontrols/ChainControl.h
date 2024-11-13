//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"
#include "ChainControlData.h"
#include "ChainControlDataSystem.h"

/**
 * Control that additionally provides the capability to produce/consume data values,
 * to allow control operations to be chained together.
 */
class ChainControl : public Control
{
public:
  static InputParameters validParams();

  ChainControl(const InputParameters & parameters);

  /**
   * Initialization that occurs in \c ChainControlSetupAction, right before the dependencies
   * are added.
   *
   * Note that the \c initialSetup() method is executed after all
   * actions are completed, so it is too late to add control data dependencies.
   */
  virtual void init() {}

  /**
   * Returns the ChainControls that must run before this one
   */
  const std::vector<std::string> & getChainControlDataDependencies() const
  {
    return _control_data_depends_on;
  }

protected:
  /**
   * Declares chain control data with the given name and type
   *
   * @tparam T          Type of chain control data
   * @param data_name   Chain control data name
   * @param apply_object_prefix   If true, apply the object name as a prefix to the data name
   */
  template <typename T>
  T & declareChainControlData(const std::string & data_name, bool apply_object_prefix = true);

  /**
   * Get a reference to control data that are specified in the input parameter 'param_name'
   *
   * @tparam T      Type of chain control data
   * @param param   Parameter containing chain control data name
   */
  template <typename T>
  const T & getChainControlData(const std::string & param);

  /**
   * Get a reference to control data value from a previous time step that is specified in the input
   * parameter 'param_name'
   *
   * @tparam T      Type of chain control data
   * @param param   Parameter containing chain control data name
   */
  template <typename T>
  const T & getChainControlDataOld(const std::string & param);

  /**
   * Get a reference to control data that are specified by 'data_name' name
   *
   * @tparam T          Type of chain control data
   * @param data_name   Chain control data name
   */
  template <typename T>
  const T & getChainControlDataByName(const std::string & data_name);

  /**
   * Get a reference to control data value from previous time step that is specified by 'data_name'
   * name
   *
   * @tparam T          Type of chain control data
   * @param data_name   Chain control data name
   */
  template <typename T>
  const T & getChainControlDataOldByName(const std::string & data_name);

  /**
   * Adds a chain control data dependency into the list
   *
   * @param data_name   Name of chain control data that this control depends upon
   */
  void addChainControlDataDependency(const std::string & data_name);

  /// List of chain control data that this control depends upon
  std::vector<std::string> _control_data_depends_on;
};

template <typename T>
T &
ChainControl::declareChainControlData(const std::string & data_name, bool apply_object_prefix)
{
  const std::string full_data_name = (apply_object_prefix ? name() + ":" : "") + data_name;
  auto & data =
      getMooseApp().getChainControlDataSystem().declareChainControlData<T>(full_data_name, *this);
  return data.set();
}

template <typename T>
const T &
ChainControl::getChainControlData(const std::string & param)
{
  return getChainControlDataByName<T>(getParam<std::string>(param));
}

template <typename T>
const T &
ChainControl::getChainControlDataOld(const std::string & param)
{
  return getChainControlDataOldByName<T>(getParam<std::string>(param));
}

template <typename T>
const T &
ChainControl::getChainControlDataByName(const std::string & data_name)
{
  auto & data = getMooseApp().getChainControlDataSystem().getChainControlData<T>(data_name);

  addChainControlDataDependency(data_name);

  return data.get();
}

template <typename T>
const T &
ChainControl::getChainControlDataOldByName(const std::string & data_name)
{
  auto & data = getMooseApp().getChainControlDataSystem().getChainControlData<T>(data_name);

  return data.getOld();
}
