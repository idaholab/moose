//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataReporter.h"

#include "MooseUtils.h"

registerMooseObject("MooseApp", RestartableDataReporter);

InputParameters
RestartableDataReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reports restartable data and restartable meta data.");

  MultiMooseEnum entries("value type declared loaded stored has_context", "value type");
  params.addParam<MultiMooseEnum>(
      "entries", entries, "The entries to output for each restartable value");

  params.addParam<bool>("allow_unimplemented",
                        false,
                        "Set to true to allow the empty output of data that does not have a JSON "
                        "output implementation");

  params.addParam<std::string>(
      "map", "", "The data map to use; if unset, use system restartable data");

  params.addParam<std::vector<std::string>>(
      "include",
      {},
      "The data name patterns to include (* for matching all, ? for matching a single character)");
  params.addParam<std::vector<std::string>>(
      "exclude",
      {},
      "The data name patterns to exclude (* for matching all, ? for matching a single character)");

  return params;
}

RestartableDataReporter::RestartableDataReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _data_params(getDataParams()),
    _allow_unimplemented(getParam<bool>("allow_unimplemented")),
    _include(getParam<std::vector<std::string>>("include")),
    _exclude(getParam<std::vector<std::string>>("exclude")),
    _values(declareValueByName<std::map<std::string, RestartableDataReporter::Value>>(
        "values",
        getParam<std::string>("map").size() ? REPORTER_MODE_ROOT : REPORTER_MODE_DISTRIBUTED)),
    _data_map(getParam<std::string>("map").size()
                  ? _app.getRestartableDataMap(getParam<std::string>("map"))
                  : _app.getRestartableData()[0])
{
}

RestartableDataValue::StoreJSONParams
RestartableDataReporter::getDataParams() const
{
  const auto & entries = getParam<MultiMooseEnum>("entries");

  RestartableDataValue::StoreJSONParams params;
  params.value = entries.isValueSet("value");
  params.type = entries.isValueSet("type");
  params.name = false;
  params.declared = entries.isValueSet("declared");
  params.loaded = entries.isValueSet("loaded");
  params.stored = entries.isValueSet("stored");
  params.has_context = entries.isValueSet("has_context");

  return params;
}

void
RestartableDataReporter::execute()
{
  _values.clear();

  for (const auto & value : _data_map)
  {
    mooseAssert(!_values.count(value.name()), "Non-unique name");

    // Don't output JSON entries
    if (typeid(nlohmann::json) == value.typeId())
      continue;
    // Don't output Reporter data due to recursion
    if (MooseUtils::globCompare(value.name(),
                                ReporterName::REPORTER_RESTARTABLE_DATA_PREFIX + "/*"))
      continue;
    // Does not match include patterns
    if (_include.size() && std::find_if(_include.begin(),
                                        _include.end(),
                                        [&value](const auto & pattern) {
                                          return MooseUtils::globCompare(value.name(), pattern);
                                        }) == _include.end())
      continue;
    // Matches exclude patterns
    if (std::find_if(_exclude.begin(),
                     _exclude.end(),
                     [&value](const auto & pattern)
                     { return MooseUtils::globCompare(value.name(), pattern); }) != _exclude.end())
      continue;

    RestartableDataReporter::Value entry;
    entry.value = &value;
    entry.params = _data_params;

    if (!value.hasStoreJSON())
    {
      if (!_allow_unimplemented)
        mooseError("The method for outputting restartable data of type '",
                   value.type(),
                   "' is not implemented.\n\nTo omit data values that are not able to be "
                   "output, set Reporters/",
                   name(),
                   "/"
                   "allow_unimplemented=true,\nor skip the data with Reporters/",
                   name(),
                   "/[include/exclude].");
      entry.params.value = false;
    }

    _values.emplace(value.name(), entry);
  }
}

void
to_json(nlohmann::json & json, const RestartableDataReporter::Value & value)
{
  mooseAssert(value.value, "Not set");
  value.value->store(json, value.params);
}
