//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <map>
#include <string>

#include "hit/hit.h"

#include "MooseUtils.h"

#define combineParamNames1(X, Y) X##Y
#define combineParamNames(X, Y) combineParamNames1(X, Y)

/// Macro for registering a scalar parameter using the setScalarValue helper
#define registerScalarParameter(type)                                                              \
  static char combineParamNames(dummyvar_reg_param, __COUNTER__) =                                 \
      Moose::ParameterRegistry::get().add<type>(                                                   \
          [](type & value, const hit::Field & field)                                               \
          { Moose::ParameterRegistration::setScalarValue<type>(value, field); })
/// Macro for registering a vector parameter using the setVectorValue helper
#define registerVectorParameter(type)                                                              \
  static char combineParamNames(dummyvar_reg_param, __COUNTER__) =                                 \
      Moose::ParameterRegistry::get().add<std::vector<type>>(                                      \
          [](std::vector<type> & value, const hit::Field & field)                                  \
          { Moose::ParameterRegistration::setVectorValue<type>(value, field); })
/// Macro for registering a double vector parameter using the setDoubleVectorValue helper
#define registerDoubleVectorParameter(type)                                                        \
  static char combineParamNames(dummyvar_reg_param, __COUNTER__) =                                 \
      Moose::ParameterRegistry::get().add<std::vector<std::vector<type>>>(                         \
          [](std::vector<std::vector<type>> & value, const hit::Field & field)                     \
          { Moose::ParameterRegistration::setDoubleVectorValue<type>(value, field); })
/// Macro for registering a double vector parameter using the setTripleVectorValue helper
#define registerTripleVectorParameter(type)                                                        \
  static char combineParamNames(dummyvar_reg_param, __COUNTER__) =                                 \
      Moose::ParameterRegistry::get().add<std::vector<std::vector<std::vector<type>>>>(            \
          [](std::vector<std::vector<std::vector<type>>> & value, const hit::Field & field)        \
          { Moose::ParameterRegistration::setTripleVectorValue<type>(value, field); })
/// Macro for registering a scalar, vector, double vector, and triple vector parameter
#define registerParameter(type)                                                                    \
  registerScalarParameter(type);                                                                   \
  registerVectorParameter(type);                                                                   \
  registerDoubleVectorParameter(type);                                                             \
  registerTripleVectorParameter(type)
/// Macro for registering a map parameter of the given type using the setMapValue helper
#define registerMapParameter(key_type, value_type)                                                 \
  static char combineParamNames(dummyvar_reg_param, __COUNTER__) =                                 \
      Moose::ParameterRegistry::get().add<std::map<key_type, value_type>>(                         \
          [](std::map<key_type, value_type> & value, const hit::Field & field)                     \
          { Moose::ParameterRegistration::setMapValue<key_type, value_type>(value, field); })

namespace Moose::ParameterRegistration
{

/**
 * Converts the given field node into a scalar of the given type.
 */
template <class T>
void setScalarValue(T & value, const hit::Field & field);

/**
 * Converts the given field node into a vector of the given type.
 */
template <class T>
void setVectorValue(std::vector<T> & value, const hit::Field & field);

/**
 * Converts the given field node into a vector-of-vectors of the given type.
 */
template <class T>
void setDoubleVectorValue(std::vector<std::vector<T>> & value, const hit::Field & field);

/**
 * Converts the given field node into a triple-indexed vector of the given type.
 */
template <class T>
void setTripleVectorValue(std::vector<std::vector<std::vector<T>>> & value,
                          const hit::Field & field);

/**
 * Converts the given field node into a map with the given types.
 */
template <class Key, class Value>
void setMapValue(std::map<Key, Value> & value, const hit::Field & field);

/// setScalarValue specialiation for bool
template <>
void setScalarValue(bool & value, const hit::Field & field);

template <class T>
void
setScalarValue(T & value, const hit::Field & field)
{
  const auto strval = field.param<std::string>();
  if constexpr (std::is_base_of_v<std::string, T>)
  {
    value = strval;
  }
  else
  {
    if (!MooseUtils::convert<T>(strval, value, false))
      throw std::invalid_argument("invalid syntax for " + MooseUtils::prettyCppType<T>() +
                                  " parameter: " + field.fullpath() + "='" + strval + "'");
  }
}

template <class T>
void
setVectorValue(std::vector<T> & value, const hit::Field & field)
{
  value.clear();
  const auto base_values = field.param<std::vector<std::string>>();
  if constexpr (std::is_base_of_v<std::string, T>)
  {
    std::copy(base_values.begin(), base_values.end(), std::back_inserter(value));
  }
  else
  {
    value.resize(base_values.size());
    for (const auto i : index_range(base_values))
    {
      if constexpr (std::is_same_v<bool, T>)
      {
        if (base_values[i] == "1")
        {
          value[i] = true;
          continue;
        }
        else if (base_values[i] == "0")
        {
          value[i] = false;
          continue;
        }
        else if (bool bool_val; hit::toBool(base_values[i], &bool_val))
        {
          value[i] = bool_val;
          continue;
        }
      }
      else
      {
        if (MooseUtils::convert<T>(base_values[i], value[i], false))
          continue;
      }

      throw std::invalid_argument("invalid syntax for " + MooseUtils::prettyCppType<T>() +
                                  " vector parameter: " + field.fullpath() + "[" +
                                  std::to_string(i) + "]='" + base_values[i] + "'");
    }
  }
}

template <class T>
void
setDoubleVectorValue(std::vector<std::vector<T>> & value, const hit::Field & field)
{
  value.clear();
  const auto strval = MooseUtils::trim(field.param<std::string>());
  if (strval.empty())
    return;

  // split vector on ";" (the substrings are _not_ of type T yet)
  // The zero length here is intentional, as we want something like:
  // "abc; 123;" -> ["abc", "123", ""]
  const auto tokens = MooseUtils::split(strval, ";");

  value.resize(tokens.size());
  for (const auto i : index_range(tokens))
  {
    const auto token = MooseUtils::trim(tokens[i]);
    if (!MooseUtils::tokenizeAndConvert<T>(token, value[i]))
      throw std::invalid_argument("invalid syntax for " + MooseUtils::prettyCppType<T>() +
                                  " double vector parameter: " + field.fullpath() + "[" +
                                  std::to_string(i) + "]='" + token + "'");
  }
}

template <class T>
void
setTripleVectorValue(std::vector<std::vector<std::vector<T>>> & value, const hit::Field & field)
{
  value.clear();
  const auto value_string = field.param<std::string>();
  if (value_string.find_first_not_of(' ', 0) == std::string::npos)
    return;

  // Add a space between neighboring delim's, before the first delim if nothing is ahead of it, and
  // after the last delim if nothing is behind it.
  std::string buffer;
  buffer.push_back(value_string[0]);
  if (buffer[0] == '|' || buffer[0] == ';')
    buffer = ' ' + buffer;
  for (std::string::size_type i = 1; i < value_string.size(); i++)
  {
    const auto val = value_string[i];
    const auto last_val = value_string[i - 1];
    if ((last_val == '|' || last_val == ';') && (val == '|' || val == ';'))
      buffer.push_back(' ');
    buffer.push_back(val);
  }
  if (buffer.back() == '|' || buffer.back() == ';')
    buffer.push_back(' ');

  // split vector at delim | to get a series of 2D subvectors
  std::vector<std::string> outer_tokens;
  MooseUtils::tokenize(buffer, outer_tokens, 1, "|");
  value.resize(outer_tokens.size());
  for (const auto i : index_range(outer_tokens))
  {
    const auto & inner_token = outer_tokens[i];
    auto & inner_value = value[i];

    // Identify empty subvector first
    if (inner_token.find_first_not_of(' ', 0) == std::string::npos)
    {
      mooseAssert(inner_value.empty(), "Should be empty");
      continue;
    }

    // split each 2D subvector at delim ; to get 1D sub-subvectors
    // NOTE: the 1D sub-subvectors are _not_ of type T yet
    std::vector<std::string> inner_tokenized;
    MooseUtils::tokenize(inner_token, inner_tokenized, 1, ";");
    inner_value.resize(inner_tokenized.size());
    for (const auto j : index_range(inner_tokenized))
    {
      const auto token = MooseUtils::trim(inner_tokenized[j]);
      if (!MooseUtils::tokenizeAndConvert<T>(token, inner_value[j]))
        throw std::invalid_argument("invalid syntax for " + MooseUtils::prettyCppType<T>() +
                                    " triple vector parameter: " + field.fullpath() + "[" +
                                    std::to_string(i) + "][" + std::to_string(j) + "]='" + token +
                                    "'");
    }
  }
}

template <class Key, class Value>
void
setMapValue(std::map<Key, Value> & value, const hit::Field & field)
{
  value.clear();

  const auto string_vec = field.param<std::vector<std::string>>();
  auto it = string_vec.begin();
  while (it != string_vec.end())
  {
    const auto & string_key = *(it++);
    if (it == string_vec.end())
      throw std::invalid_argument(
          "odd number of entries for map parameter '" + field.fullpath() +
          "'; there must be an even number or else you will end up with a key without a value");
    const auto & string_value = *(it++);

    std::pair<Key, Value> pr;

    // convert key
    if (!MooseUtils::convert<Key>(string_key, pr.first, false))
      throw std::invalid_argument("invalid " + MooseUtils::prettyCppType<Key>() +
                                  " syntax for map parameter '" + field.fullpath() + "' key: '" +
                                  string_key + "'");

    // convert value
    if (!MooseUtils::convert<Value>(string_value, pr.second, false))
      throw std::invalid_argument("invalid " + MooseUtils::prettyCppType<Value>() +
                                  " syntax for map parameter '" + field.fullpath() + "' value: '" +
                                  string_value + "'");
    // attempt insert
    if (!value.insert(std::move(pr)).second)
      throw std::invalid_argument("duplicate entry for map parameter: '" + field.fullpath() +
                                  "'; key '" + string_key + "' appears multiple times");
  }
}

} // end of namespace Moose::ParameterRegistration
