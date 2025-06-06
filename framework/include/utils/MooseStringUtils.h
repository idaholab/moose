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
inline std::string
toUpper(const std::string & name)
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  return upper;
}

/**
 * Convert supplied string to lower case.
 * @params name The string to convert upper case.
 */
inline std::string
toLower(const std::string & name)
{
  std::string lower(name);
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  return lower;
}

}
