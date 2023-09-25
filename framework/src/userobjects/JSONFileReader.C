//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JSONFileReader.h"

#include "MooseUtils.h"
#include "json.h"

registerMooseObject("MooseApp", JSONFileReader);

InputParameters
JSONFileReader::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  // Add parameters
  params.addRequiredParam<FileName>("filename", "The path to the file including its name");
  // we run this object once at the initialization by default
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.addClassDescription("Loads a JSON file and makes it content available to consumers");
  return params;
}

JSONFileReader::JSONFileReader(const InputParameters & parameters)
  : GeneralUserObject(parameters), _filename(getParam<FileName>("filename"))
{
  read(_filename);
}

void
JSONFileReader::read(const FileName & filename)
{
  MooseUtils::checkFileReadable(filename);
  // Read the JSON database
  std::ifstream jsondata(filename);
  jsondata >> _root;
}

void
JSONFileReader::getScalar(const std::string & scalar_name, Real & scalar) const
{
  if (!_root.contains(scalar_name))
    mooseError("Attempted to get '",
               scalar_name,
               "' but the JSON file does not contain this key directly at the root level");
  scalar = getReal(_root[scalar_name]);
}

void
JSONFileReader::getScalar(const std::vector<std::string> & scalar_keys, Real & scalar) const
{
  if (!scalar_keys.size())
    mooseError("There should be at least one key to retrieve a value from the JSON");

  // traverse the JSON tree
  auto * current_node = &_root[scalar_keys[0]];
  for (const auto key_index : index_range(scalar_keys))
  {
    if (key_index == scalar_keys.size() - 1)
    {
      scalar = getReal(*current_node);
      break;
    }
    current_node = &(*current_node)[scalar_keys[key_index + 1]];
  }
}

void
JSONFileReader::getVector(const std::string & vector_name, std::vector<Real> & vector_to_fill) const
{
  if (!_root.contains(vector_name))
    mooseError("Attempted to get '",
               vector_name,
               "' but the JSON file does not contain this key at the root level");
  const auto num_items = _root[vector_name].size();
  vector_to_fill.resize(num_items);
  for (const auto index : make_range(num_items))
    vector_to_fill[index] = getReal(_root[vector_name][index]);
}

void
JSONFileReader::getVector(const std::vector<std::string> & vector_keys,
                          std::vector<Real> & vector_to_fill) const
{
  if (!vector_keys.size())
    mooseError("There should be at least one key to retrieve a value from the JSON");

  // traverse the JSON tree
  auto * current_node = &_root[vector_keys[0]];
  for (const auto key_index : index_range(vector_keys))
  {
    if (key_index == vector_keys.size() - 1)
    {
      if (!current_node->is_array())
        mooseError("Cannot retrieve a vector from JSON node",
                   *current_node,
                   "obtained with last key",
                   vector_keys[key_index]);
      const auto num_items = current_node->size();
      vector_to_fill.resize(num_items);
      for (const auto index : make_range(num_items))
        vector_to_fill[index] = getReal((*current_node)[index]);
      return;
    }
    if (current_node->is_array())
      mooseError("Cannot obtain nested JSON item with key",
                 vector_keys[key_index + 1],
                 "because the current item is an array:",
                 *current_node);
    current_node = &(*current_node)[vector_keys[key_index + 1]];
  }
}

Real
JSONFileReader::getReal(const nlohmann::json & node) const
{
  if (node.is_string())
    return MooseUtils::convert<Real>(node);
  return node;
}
