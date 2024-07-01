//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace Moose
{
/**
 * Create a map from two vectors
 * @param keys the vector of the keys
 * @param values the vector of the values
 * @tparam T the type of the keys
 * @tparam C the type of the values
 */
template <typename T, typename C>
std::map<T, C> createMapFromVectors(const std::vector<T> & keys, const std::vector<C> & values);

/**
 * Create a map from a vector of keys and MultiMooseEnum acting as a vector
 * @param keys the vector of the keys
 * @param values the MultiMooseEnum acting as a vector of the values
 * @tparam T the type of the keys
 */
template <typename T>
std::map<T, MooseEnum> createMapFromVectorAndMultiMooseEnum(const std::vector<T> & keys,
                                                            const MultiMooseEnum & values);

template <typename T, typename C>
std::map<T, C>
createMapFromVectors(const std::vector<T> & keys, const std::vector<C> & values)
{
  std::map<T, C> map;
  mooseAssert(keys.size() == values.size(),
              "Map should be made from keys (" + std::to_string(keys.size()) + ") and values (" +
                  std::to_string(values.size()) + ") of the same size");

  // No values have been specified.
  if (!values.size())
  {
    return map;
  }
  std::transform(keys.begin(),
                 keys.end(),
                 values.begin(),
                 std::inserter(map, map.end()),
                 [](const T & a, const C & b) { return std::make_pair(a, b); });
  return map;
}

template <typename T>
std::map<T, MooseEnum>
createMapFromVectorAndMultiMooseEnum(const std::vector<T> & keys, const MultiMooseEnum & values)
{
  std::map<T, MooseEnum> map;
  mooseAssert(keys.size() == values.size(),
              "Map should be made from keys and values of the same size");
  // No values have been specified. We cant form a map of empty MooseEnum
  if (!values.size())
    return map;
  std::transform(keys.begin(),
                 keys.end(),
                 values.begin(),
                 std::inserter(map, map.end()),
                 [values](const T & a, const MooseEnumItem & b)
                 {
                   // Create a MooseEnum from the available values in the MultiMooseEnum and an
                   // actual current active item from that same MultiMooseEnum
                   MooseEnum single_value(values.getRawNames(), b.name());
                   return std::make_pair(a, single_value);
                 });
  return map;
}
}
