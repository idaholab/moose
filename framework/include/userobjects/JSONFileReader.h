//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * User object that reads a JSON file and makes its data available to other objects
 */
class JSONFileReader : public GeneralUserObject
{
public:
  static InputParameters validParams();

  JSONFileReader(const InputParameters & parameters);

  /// Required implementation of a pure virtual function (not used)
  virtual void initialize() override {}

  /// Required implementation of a pure virtual function (not used)
  virtual void finalize() override {}

  /// Read the file again
  virtual void execute() override { read(_filename); }

  /// Getters for scalar values
  /**
   * Get a scalar that is directly indexed at the root of the JSON file/tree
   * @param scalar_name the name of the desired scalar
   * @param scalar reference to the scalar that will be set with the scalar in the JSON
   */
  template<typename T>
  void getScalar(const std::string & scalar_name, T & scalar) const
  {
    if (!_root.contains(scalar_name))
      mooseError("Attempted to get '",
                 scalar_name,
                 "' but the JSON file does not contain this key directly at the root level");
    scalar = _root[scalar_name].get<T>();
  }

  /**
   * Get a scalar in the JSON file/tree using the keys in the 'scalar_keys' one by one to
   * traverse the JSON tree down to the requested scalar
   * @param scalar_keys the keys in descending order to access the scalar
   * @param scalar reference to the scalar that will be set with the scalar in the JSON
   */
  template<typename T>
  void getScalar(const std::vector<std::string> & scalar_keys, T & scalar) const
  {
    if (!scalar_keys.size())
      mooseError("There should be at least one key to retrieve a value from the JSON");

    // traverse the JSON tree
    auto * current_node = &_root[scalar_keys[0]];
    for (const auto key_index : index_range(scalar_keys))
    {
      if (key_index == scalar_keys.size() - 1)
      {
        scalar = current_node->get<T>();
        break;
      }
      current_node = &(*current_node)[scalar_keys[key_index + 1]];
    }
  }
  /// Getter for vector values
  /**
   * Get a vector that is directly indexed at the root of the JSON file/tree
   * @param vector_name the name of the desired vector
   * @param vector_to_fill reference to the vector that will be set with the vector in the JSON
   */
  template<typename T>
  void getVector(const std::string & vector_name, std::vector<T> & vector_to_fill) const
  {
  if (!_root.contains(vector_name))
    mooseError("Attempted to get '",
               vector_name,
               "' but the JSON file does not contain this key at the root level");
  vector_to_fill.clear();
  for (const auto& item : _root[vector_name])
    vector_to_fill.push_back(item.get<T>());
  }
  /**
   * Get a vector in the JSON file/tree using the keys in the 'vector_keys' vector one by one
   * to traverse the JSON tree down to the requested vector
   * @param vector_keys the keys in descending order to access the vector
   * @param vector_to_fill reference to the vector that will be set with the vector in the JSON
   */
  template<typename T>
  void getVector(const std::vector<std::string>& vector_keys, std::vector<T>& vector_to_fill) const
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
        vector_to_fill.clear();
        for (const auto& item : *current_node)
          vector_to_fill.push_back(item.get<T>());
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

private:
  /**
   * Read the JSON file and load it into _root
   * @param filename the name of the file
   */
  void read(const FileName & filename);

  /// Database filename
  const FileName & _filename;
  /// JSON data
  nlohmann::json _root;
};
