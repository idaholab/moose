//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

// The capabilities library (capabilities target in moose.mk) uses this
// utility for parsing. We don't want to include libmesh libraries in
// this library as the test harness uses it. However, in the convert
// method (heavily used by the parser), we really want to take advantage
// of libMesh::demangle() for useful error messages. So this lets us
// still use pretty demangling when used by MOOSE but not by the
// capabilities library (which is probably ok...)
#ifndef MOOSESTRINGUTILS_NO_LIBMESH
#include "libmesh/libmesh_common.h"
#endif

/*
 * This must stay a header-only utility! It is used in the capabilities python module and
 * we do not want to link against any MOOSE libs.
 */
namespace MooseUtils
{
/**
 * Standard scripting language trim function
 */
inline std::string
trim(const std::string & str, const std::string & white_space = " \t\n\v\f\r")
{
  const auto begin = str.find_first_not_of(white_space);
  if (begin == std::string::npos)
    return ""; // no content
  const auto end = str.find_last_not_of(white_space);
  return str.substr(begin, end - begin + 1);
}

/**
 * This function will split the passed in string on a set of delimiters appending the substrings
 * to the passed in vector.  The delimiters default to "/" but may be supplied as well.  In
 * addition if min_len is supplied, the minimum token length will be >= than the supplied
 * value. T should be std::string or a MOOSE derived string class.
 */
template <typename T>
void
tokenize(const std::string & str,
         std::vector<T> & elements,
         unsigned int min_len = 1,
         const std::string & delims = "/")
{
  elements.clear();

  std::string::size_type last_pos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));

  while (last_pos != std::string::npos)
  {
    elements.push_back(str.substr(last_pos, pos - last_pos));
    // skip delims between tokens
    last_pos = str.find_first_not_of(delims, pos);
    if (last_pos == std::string::npos)
      break;
    pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));
  }
}

/**
 * Takes the string representation of a value and converts it to the value.
 *
 * For standard numeric types, this gets around the deficiencies in the STL
 * stoi and stod methods where they might successfully convert part of a string
 * to a number when we'd instead prefer to get a failure.
 *
 * For string and string-derived types, this does a direct copy and does
 * not utilize a stream.
 *
 * For all other types, this uses the stringstream >> operator to fill
 * the value.
 *
 * @param str The string to convert from
 * @param value The typed value to fill
 * @param throw_on_failure If true, throw a std::invalid_argument on failure
 * @return Whether or not the conversion succeeded
 */
template <class T>
bool
convert(const std::string & str, T & value, const bool throw_on_failure)
{
  // Special case for numeric values, also handling range checking
  if constexpr (std::is_same_v<short int, T> || std::is_same_v<unsigned short int, T> ||
                std::is_same_v<int, T> || std::is_same_v<unsigned int, T> ||
                std::is_same_v<long int, T> || std::is_same_v<unsigned long int, T> ||
                std::is_same_v<long long int, T> || std::is_same_v<unsigned long long int, T>)
  {
    // Try read a double and try to cast it to an int
    long double double_val;
    std::stringstream double_ss(str);
    double_ss >> double_val;

    if (!double_ss.fail() && double_ss.eof())
    {
      // on arm64 the long double does not have sufficient precision
      std::stringstream int_ss(str);
      const bool use_int = !(int_ss >> value).fail() && int_ss.eof();

      // Check to see if it's an integer and thus within range of an integer
      if (double_val == static_cast<long double>(static_cast<T>(double_val)))
      {
        if (!use_int)
          value = static_cast<T>(double_val);
        return true;
      }
    }
  }
  // Non numeric values
  else
  {
    // string or derived string: direct copy
    if constexpr (std::is_base_of_v<std::string, T>)
    {
      value = str;
      return true;
    }
    // non-string or numeric, use stringstream >>
    else
    {
      std::stringstream ss(trim(str));
      if (!(ss >> value).fail() && ss.eof())
        return true;
    }
  }

  if (throw_on_failure)
  {
    std::string error = "Unable to convert '" + str + "' to type ";
#ifdef MOOSESTRINGUTILS_NO_LIBMESH
    error += typeid(T).name();
#else
    error += libMesh::demangle(typeid(T).name());
#endif
    throw std::invalid_argument(error);
  }

  return false;
}

/**
 *  tokenizeAndConvert splits a string using delimiter and then converts to type T.
 *  If the conversion fails tokenizeAndConvert returns false, otherwise true.
 */
template <typename T>
bool
tokenizeAndConvert(const std::string & str,
                   std::vector<T> & tokenized_vector,
                   const std::string & delimiter = " \t\n\v\f\r")
{
  std::vector<std::string> tokens;
  MooseUtils::tokenize(str, tokens, 1, delimiter);
  tokenized_vector.resize(tokens.size());
  for (std::size_t i = 0; i < tokens.size(); ++i)
    if (!convert<T>(tokens[i], tokenized_vector[i], false))
      return false;
  return true;
}

/**
 * Convert supplied string to upper case.
 * @params name The string to convert upper case.
 */
inline std::string
toUpper(std::string name)
{
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  return name;
}

/**
 * Convert supplied string to lower case.
 * @params name The string to convert upper case.
 */
inline std::string
toLower(std::string name)
{
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return name;
}

/**
 * Concatenates \p value into a single string separated by \p separator
 */
inline std::string
stringJoin(const std::vector<std::string> & values, const std::string & separator = " ")
{
  std::string combined;
  for (const auto & value : values)
    combined += value + separator;
  if (values.size())
    combined = combined.substr(0, combined.size() - separator.size());
  return combined;
}
}
