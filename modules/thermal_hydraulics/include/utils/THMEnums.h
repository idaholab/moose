//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <algorithm>
#include "MooseEnum.h"

namespace THM
{

/**
 * Converts a string to an enum
 *
 * This template is designed to be specialized and use the other version of this
 * function in conjunction with the correct map.
 *
 * @tparam    T          enum type
 * @param[in] s          string to convert
 */
template <typename T>
T stringToEnum(const std::string & s);

/**
 * Converts a string to an enum using a map of string to enum
 *
 * @tparam    T          enum type
 * @param[in] s          string to convert
 * @param[in] enum_map   map of string to enum
 */
template <typename T>
T stringToEnum(const std::string & s, const std::map<std::string, T> & enum_map);

/**
 * Gets MooseEnum corresponding to an enum, using a map of string to enum
 *
 * @tparam    T             enum type
 * @param[in] default_key   key corresponding to default value
 * @param[in] enum_map      map of string to enum
 */
template <typename T>
MooseEnum getMooseEnum(const std::string & default_key, const std::map<std::string, T> & enum_map);
}

template <typename T>
T
THM::stringToEnum(const std::string & s, const std::map<std::string, T> & enum_map)
{
  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!enum_map.count(upper))
    return static_cast<T>(-100);
  else
    return enum_map.at(upper);
}

template <typename T>
MooseEnum
THM::getMooseEnum(const std::string & default_key, const std::map<std::string, T> & enum_map)
{
  std::string keys_string;
  for (typename std::map<std::string, T>::const_iterator it = enum_map.begin();
       it != enum_map.end();
       it++)
    if (it == enum_map.begin())
      keys_string += it->first;
    else
      keys_string += " " + it->first;

  return MooseEnum(keys_string, default_key, true);
}
