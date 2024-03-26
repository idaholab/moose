//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * This file should contain utility functions that have no dependencies besides the C++ standard
 * library (and header-only stuff).
 */

#include "InfixIterator.h"

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <algorithm>
#include <ostream>
#include <iterator>

namespace MooseUtils
{

/// Replaces all occurences of from in str with to and returns the result.
std::string replaceAll(std::string str, const std::string & from, const std::string & to);

/**
 * This function will escape all of the standard C++ escape characters so that they can be printed.
 * The
 * passed in parameter is modified in place
 */
void escape(std::string & str);

/**
 * Standard scripting language trim function
 */
std::string trim(const std::string & str, const std::string & white_space = " \t\n\v\f\r");

/**
 * Python like join function for strings.
 */
template <typename T>
std::string
join(const T & strings, const std::string & delimiter)
{
  std::ostringstream oss;
  std::copy(
      strings.begin(), strings.end(), infix_ostream_iterator<std::string>(oss, delimiter.c_str()));
  return oss.str();
}

/**
 * Python like split functions for strings.
 *
 * NOTE: This is similar to the tokenize function, but it maintains empty items, which tokenize does
 *       not. For example, "foo;bar;;" becomes {"foo", "bar", "", ""}.
 */
std::vector<std::string> split(const std::string & str,
                               const std::string & delimiter,
                               std::size_t max_count = std::numeric_limits<std::size_t>::max());
std::vector<std::string> rsplit(const std::string & str,
                                const std::string & delimiter,
                                std::size_t max_count = std::numeric_limits<std::size_t>::max());

/**
 * This routine is a simple helper function for searching a map by values instead of keys
 */
template <typename T1, typename T2>
bool
doesMapContainValue(const std::map<T1, T2> & the_map, const T2 & value)
{
  for (typename std::map<T1, T2>::const_iterator iter = the_map.begin(); iter != the_map.end();
       ++iter)
    if (iter->second == value)
      return true;
  return false;
}

/**
 * This function will split the passed in string on a set of delimiters appending the substrings
 * to the passed in vector.  The delimiters default to "/" but may be supplied as well.  In
 * addition if min_len is supplied, the minimum token length will be greater than the supplied
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
  for (unsigned int j = 0; j < tokens.size(); ++j)
  {
    std::stringstream ss(trim(tokens[j]));
    // we have to make sure that the conversion succeeded _and_ that the string
    // was fully read to avoid situations like [conversion to Real] 3.0abc to work
    if ((ss >> tokenized_vector[j]).fail() || !ss.eof())
      return false;
  }
  return true;
}

/**
 * Convert supplied string to upper case.
 * @params name The string to convert upper case.
 */
std::string toUpper(const std::string & name);

/**
 * Convert supplied string to lower case.
 * @params name The string to convert upper case.
 */
std::string toLower(const std::string & name);

} // namespace MooseUtils
