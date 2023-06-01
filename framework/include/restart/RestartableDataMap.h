//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RestartableData.h"

// C++ includes
#include <memory>
#include <vector>
#include <unordered_map>

/**
 * Ordered storage for RestartableData.
 */
class RestartableDataMap
{
public:
  /**
   * Adds the restartable data \p data to the map
   */
  RestartableDataValue & addData(std::unique_ptr<RestartableDataValue> data);

  /**
   * @returns Whether or not data with the name \p name is added
   */
  bool hasData(const std::string & name) const;

  /**
   * Tries to find data with the name \p name; returns nullptr if not found
   */
  ///@{
  const RestartableDataValue * findData(const std::string & name) const;
  RestartableDataValue * findData(const std::string & name);
  ///@}

  /**
   * @returns The data with the name \p name
   */
  RestartableDataValue & data(const std::string & name);

  /**
   * Begin and end iterators to the data
   */
  ///@{
  auto begin() { return _data.begin(); }
  auto end() { return _data.end(); }
  const auto begin() const { return _data.begin(); }
  const auto end() const { return _data.end(); }
  ///@}

  /**
   * @returns The size of registered data
   */
  auto size() const { return _data.size(); }
  /**
   * @returns Whether or not there is no registered data
   */
  auto empty() const { return _data.empty(); }

private:
  /// The registered data
  std::vector<std::unique_ptr<RestartableDataValue>> _data;
  /// Mapping from data name -> index in \p _data for quick indexing
  std::unordered_map<std::string, std::size_t> _name_to_data_index;
};
