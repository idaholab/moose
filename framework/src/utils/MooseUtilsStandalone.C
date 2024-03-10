//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * This file should contain utility functions that have no dependencies besides the C++ standard
 * library.
 */

#include "MooseUtilsStandalone.h"

namespace MooseUtils
{

std::string
replaceAll(std::string str, const std::string & from, const std::string & to)
{
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos)
  {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
  return str;
}

void
escape(std::string & str)
{
  std::map<char, std::string> escapes;
  escapes['\a'] = "\\a";
  escapes['\b'] = "\\b";
  escapes['\f'] = "\\f";
  escapes['\n'] = "\\n";
  escapes['\t'] = "\\t";
  escapes['\v'] = "\\v";
  escapes['\r'] = "\\r";

  for (const auto & it : escapes)
    for (size_t pos = 0; (pos = str.find(it.first, pos)) != std::string::npos;
         pos += it.second.size())
      str.replace(pos, 1, it.second);
}

std::string
trim(const std::string & str, const std::string & white_space)
{
  const auto begin = str.find_first_not_of(white_space);
  if (begin == std::string::npos)
    return ""; // no content
  const auto end = str.find_last_not_of(white_space);
  return str.substr(begin, end - begin + 1);
}

std::vector<std::string>
split(const std::string & str, const std::string & delimiter, std::size_t max_count)
{
  std::vector<std::string> output;
  std::size_t count = 0;
  size_t prev = 0, pos = 0;
  do
  {
    pos = str.find(delimiter, prev);
    output.push_back(str.substr(prev, pos - prev));
    prev = pos + delimiter.length();
    count += 1;
  } while (pos != std::string::npos && count < max_count);

  if (pos != std::string::npos)
    output.push_back(str.substr(prev));

  return output;
}

std::vector<std::string>
rsplit(const std::string & str, const std::string & delimiter, std::size_t max_count)
{
  std::vector<std::string> output;
  std::size_t count = 0;
  size_t prev = str.length(), pos = str.length();
  do
  {
    pos = str.rfind(delimiter, prev);
    output.insert(output.begin(), str.substr(pos + delimiter.length(), prev - pos));
    prev = pos - delimiter.length();
    count += 1;
  } while (pos != std::string::npos && pos > 0 && count < max_count);

  if (pos != std::string::npos)
    output.insert(output.begin(), str.substr(0, pos));

  return output;
}

std::string
toUpper(const std::string & name)
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  return upper;
}

std::string
toLower(const std::string & name)
{
  std::string lower(name);
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  return lower;
}

} // namespace MooseUtils
