//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControlData.h"
#include "MooseError.h"
#include "MooseUtils.h"

class ChainControl;

/**
 * System that manages ChainControls.
 *
 * To be owned by the MooseApp.
 */
class ChainControlDataSystem
{
public:
  ChainControlDataSystem(MooseApp & app);

  /**
   * Queries if the chain control data of the given name exists.
   *
   * @param[in] data_name  Chain control data name
   * @return True if the chain control data of the given name exists
   */
  bool hasChainControlData(const std::string & data_name) const;

  /**
   * Queries if the chain control data of the given name and type exists.
   *
   * @tparam T             Type of the data
   * @param[in] data_name  Chain control data name
   * @return True if the chain control data of the given name and type exists
   */
  template <typename T>
  bool hasChainControlDataOfType(const std::string & data_name) const;

  /**
   * Gets the chain control data of the given name and type and creates if it does not exist.
   *
   * If the data exists but of another type, then an error is thrown.
   *
   * @tparam T             Type of the data
   * @param[in] data_name  Chain control data name
   * @return Reference to the requested chain control data
   */
  template <typename T>
  ChainControlData<T> & getChainControlData(const std::string & data_name);

  /**
   * Declares chain control data of of the given name and type and creates if it does not exist.
   *
   * If the data has already been declared, an error is thrown; otherwise the
   * data is now set to be declared, and the declaring ChainControl is captured.
   *
   * @tparam T                  Type of the data
   * @param[in] data_name       Chain control data name
   * @param[in] chain_control   Chain control that is declaring the data
   * @return Reference to the declared chain control data
   */
  template <typename T>
  ChainControlData<T> & declareChainControlData(const std::string & data_name,
                                                ChainControl & chain_control);

  /**
   * Copies current chain control data values into old values
   */
  void copyValuesBack();

  /**
   * Gets the map of ChainControlData names to the relevant ChainControlDataBase
   */
  const std::map<std::string, std::unique_ptr<ChainControlDataBase>> &
  getChainControlDataMap() const;

private:
  /// The MooseApp that owns this system
  MooseApp & _app;

  /// Map of chain control data name to its value
  std::map<std::string, std::unique_ptr<ChainControlDataBase>> _chain_control_data_map;
};

template <typename T>
bool
ChainControlDataSystem::hasChainControlDataOfType(const std::string & data_name) const
{
  if (hasChainControlData(data_name))
    return dynamic_cast<ChainControlData<T> *>(_chain_control_data_map.at(data_name).get()) !=
           nullptr;
  else
    return false;
}

template <typename T>
ChainControlData<T> &
ChainControlDataSystem::getChainControlData(const std::string & data_name)
{
  if (hasChainControlData(data_name))
  {
    if (!hasChainControlDataOfType<T>(data_name))
      mooseError("The chain control data '",
                 data_name,
                 "' was requested with the type '",
                 MooseUtils::prettyCppType<T>(),
                 "' but has the type '",
                 _chain_control_data_map[data_name]->type(),
                 "'.");
  }
  else
    _chain_control_data_map[data_name] = std::make_unique<ChainControlData<T>>(_app, data_name);

  return static_cast<ChainControlData<T> &>(*_chain_control_data_map[data_name]);
}

template <typename T>
ChainControlData<T> &
ChainControlDataSystem::declareChainControlData(const std::string & data_name,
                                                ChainControl & chain_control)
{
  auto & data = getChainControlData<T>(data_name);
  if (!data.getDeclared())
  {
    data.setDeclared();
    data.setChainControl(chain_control);
  }
  else
    mooseError("The chain control data '", data_name, "' has already been declared.");

  return data;
}
