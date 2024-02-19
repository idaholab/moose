//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

/**
 * Reports restartable data and restartable meta data.
 */
class RestartableDataReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  RestartableDataReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

  /**
   * Helper struct for storing a single piece of restartable data.
   *
   * This helper is needed in order to get the paramaters into
   * to_json for output, so that we can change how much we output
   * via the "entries" input parameter as deisred
   */
  struct Value
  {
    const RestartableDataValue * value;
    RestartableDataValue::StoreJSONParams params;
  };

protected:
  /// The parameters to pass to the output of a single value
  const RestartableDataValue::StoreJSONParams _data_params;
  /// Whether or not to error on the output of types with unimplemented output methods
  const bool _allow_unimplemented;
  /// The include patterns to match
  const std::vector<std::string> _include;
  /// The exclude patterns to match
  const std::vector<std::string> _exclude;
  /// The values we are to output
  std::map<std::string, RestartableDataReporter::Value> & _values;
  /// The map of data that we're going to output
  const RestartableDataMap & _data_map;

private:
  /// Internal method for setting _data_params
  RestartableDataValue::StoreJSONParams getDataParams() const;
};

void to_json(nlohmann::json & json, const RestartableDataReporter::Value & value);
